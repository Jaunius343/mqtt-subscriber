#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <signal.h>
#include <argp.h>
#include <string.h>
#include <curl/curl.h>
#include <uci.h>
#include <unistd.h>

#include "argp_parser.h"
#include "helper_functions.h"
#include "mqtt_func.h"

#include "msg_log.h"
#include "emails.h"
#include "msg_event.h"
#include "uci_func.h"
#include "topics.h"
#include "data_container.h"
#include "threads.h"
#include "log.h"

int flag = 1;

int main(int argc, char **argv)
{
	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);

	struct data_container_t data_cont;
	struct arguments args	    = { 0 };
	struct smtp_settings_t smtp = { 0 };

	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_t thread;

	int rc;
	struct mosquitto *mosq;

	argp_parse(&argp, argc, argv, 0, 0, &args);

	data_cont.topics      = topic_create_array();
	data_cont.event_list  = event_list_create();
	data_cont.thread_data = thread_data_create(&mutex);
	data_cont.setup	      = setup_init();
	data_cont.smtp	      = &smtp;
	data_cont.args	      = &args;

	uci_read_file(&data_cont);
	// event_list_print(data_cont.event_list);
	// topic_print_array(data_cont.topics);

	// both file and syslog callbacks have to be under uci_read_file,
	// because uci config has log level
	if (args.daemon == 1) {
		openlog("MQTT_SUB", LOG_PID, LOG_USER);
		rc = log_add_callback(log_to_syslog, NULL, data_cont.setup->log_level);
		if (rc != 0) {
			closelog();
			exit(-1);
		}
	}

	FILE *file = file_cb_init(data_cont.setup->log_level);

	log_info("Found Events: %d, Topics: %d", data_cont.event_list->size, data_cont.topics->size);

	mosquitto_lib_init();

	mosq = mosquitto_new("subscribe-test", true, &data_cont);

	// check if user is trying to connect with username and password
	if (strlen(args.user) != 0 || strlen(args.psw) != 0) {
		if (strlen(args.user) == 0) {
			log_info("-P option requires -u");
			goto cleanup;

		} else if (strlen(args.psw) == 0) {
			log_info("-u option requires -P");
			goto cleanup;

		} else {
			log_debug("before usr/pw set");
			rc = mosquitto_username_pw_set(mosq, args.user, args.psw);
			if (rc != MOSQ_ERR_SUCCESS) {
				log_error("mosq usr/pw set status: %s", mosquitto_strerror(rc));
				goto cleanup;
			}
		}
	}

	if (*args.ca != 0 && *args.cl_cert != 0 && *args.cl_key != 0) {
		rc = mosquitto_tls_set(mosq, args.ca, NULL, args.cl_cert, args.cl_key, NULL);

		if (rc != MOSQ_ERR_SUCCESS) {
			log_error("TLS set status %d: %s", rc, mosquitto_strerror(rc));
			goto cleanup;
		}

		log_info("TLS set");

	} else if ((*args.ca != 0 && *args.ca == '\0') || (*args.cl_cert != 0 && *args.cl_cert == '\0') ||
		   (*args.cl_key != 0 && *args.cl_key == '\0')) {
		log_info("TLS usage:");
		log_info("mqtt_sub [OPTIONS] --cafile <path> --cert <path> --key <path>");
		goto cleanup;
	}

	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);

	rc = mosquitto_connect(mosq, args.host, 1883, 10);
	log_debug("mosq connect status: %s", mosquitto_strerror(rc));
	if (rc) {
		log_error("Could not connect to Broker with return code %d: %s", rc, mosquitto_strerror(rc));
		goto cleanup;
		flag = 0;
	}

	if (rc == MOSQ_ERR_SUCCESS)
		log_info("mqtt sub connected to: %s", args.host);

	mosquitto_loop_start(mosq);
	log_debug("mosquitto loop started");

	log_debug("starting event reset thread");
	if (pthread_create(&thread, NULL, &thread_reset_sent_flag, &data_cont) != 0) {
		log_error("Error creating thread");
	}

	while (flag) {
		sleep(1);
	}
	// to prevent keyboard interrupt symbols mashing with other text
	printf("\n");

	log_debug("stopping event reset thread");
	pthread_join(thread, NULL);

	log_debug("stopping mosquitto loop");
	rc = mosquitto_loop_stop(mosq, true);
	if (rc != MOSQ_ERR_SUCCESS)
		log_error("mosq loop stop status: %s", mosquitto_strerror(rc));

	log_debug("disconnecting mosquitto client");
	rc = mosquitto_disconnect(mosq);

	if (rc != MOSQ_ERR_SUCCESS)
		log_error("mosq disconnect status: %s", mosquitto_strerror(rc));

cleanup:
	log_info("starting cleanup");
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();

	topic_free_array(data_cont.topics);
	event_list_free(data_cont.event_list);
	thread_data_free(data_cont.thread_data);
	setup_free(data_cont.setup);
	msg_log_deinit();

	log_info("finished, closing...");
	fclose(file);

	if (args.daemon == 1) {
		closelog();
	}

	return 0;
}
