#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <argp.h>
#include <string.h>
#include <signal.h>

#include "argp_parser.h"
#include "helper_functions.h"
#include "msg_log.h"
#include "data_container.h"
#include "msg_event.h"
#include "log.h"

void on_connect(struct mosquitto *mosq, void *obj, int rc)
{
	struct data_container_t *data = (struct data_container_t *)obj;
	struct topic_t *topics	      = data->topics;

	if (rc) {
		log_error("Error on_connect result code: %d: %s", rc, mosquitto_strerror(rc));
		// SIGINT raise has to be before mosquitto_loop_stop to stop on first fail
		// otherwise mosquitto loop fails 5 more times before its stopped.
		raise(SIGINT);
		rc = mosquitto_loop_stop(mosq, true);
		if (rc != MOSQ_ERR_SUCCESS)
			log_error("Error mosquito loop stop inside on_connect result code: %d: %s", rc,
				  mosquitto_strerror(rc));
	}

	for (int i = 0; i < topics->size; ++i) {
		mosquitto_subscribe(mosq, NULL, topics->strings[i], 0);
		log_info("subscribed to: %s", topics->strings[i]);
	}

	// in case user provides topic from command line
	if (strlen(data->args->topic) != 0) {
		mosquitto_subscribe(mosq, NULL, data->args->topic, 0);
		log_info("subscribed to: %s", data->args->topic);
	}
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	struct data_container_t *data	= (struct data_container_t *)obj;
	struct event_list_t *event_list = data->event_list;

	char message[128];
	sprintf(message, "from \"%s\", %s", msg->topic, (char *)msg->payload);
	log_info("%s", message);

	pthread_mutex_lock(data->thread_data->mutex);

	for (int i = 0; i < event_list->size; ++i) {
		struct event_t *event = &event_list->events[i];
		int pl_size	      = 128;
		char payload_value[pl_size];
		int ret;

		ret = event_check(event, msg, payload_value, pl_size);

		if (!ret)
			continue;

		log_trace("event %d triggered", i + 1);

		event->sent = 1;
		event_form_email(event, payload_value);
		log_trace("email formed");

		// check if it's the last element of the list
		// can't do event_list->size - 1 in the first loop
		// because last element will skipped if it was not matched
		// with any previous events.
		if (i + 1 != event_list->size) {
			for (int j = i + 1; j < event_list->size; ++j) {
				// find events with matching emails
				struct event_t *event2 = &event_list->events[j];
				ret		       = event_check(event2, msg, payload_value, pl_size);

				if (!ret)
					continue;

				ret = event_compare_events(*event, *event2);

				if (!ret)
					continue;

				log_trace("subevent %d triggered", j + 1);

				char body[256];
				sprintf(body, "\nreceived value %s which is %s %s", payload_value,
					event_translate_operand(event2->operand), event2->value);

				ret	     = event_append_body(event, body);
				event2->sent = 1;
				log_trace("body appended");
			}
		}

		ret = send_email_gmail(event->email, data->smtp);
		if (ret == 0)
			log_info("email sent");
		email_cleanup_contents(&event->email);
		log_trace("-------------");
	}

	pthread_mutex_unlock(data->thread_data->mutex);

	printf("\n");
}
