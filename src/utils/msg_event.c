#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>

#include "msg_event.h"
#include "emails.h"
#include "helper_functions.h"

#define INITIAL_CAPACITY 10
#define STEP_SIZE	 10
#define EMAIL_BODY_SIZE	 1024

void string_to_lowercase(char *str)
{
	while (*str) {
		*str = tolower(*str);
		str++;
	}
}

void event_set_operand(struct event_t *event, char *operand)
{
	if (strcmp(operand, "<") == 0) {
		event->operand = EVENT_LESS;

	} else if (strcmp(operand, ">") == 0) {
		event->operand = EVENT_MORE;

	} else if (strcmp(operand, ">=") == 0) {
		event->operand = EVENT_MORE_EQUAL;

	} else if (strcmp(operand, "<=") == 0) {
		event->operand = EVENT_LESS_EQUAL;

	} else if (strcmp(operand, "==") == 0) {
		event->operand = EVENT_EQUAL;

	} else if (strcmp(operand, "!=") == 0) {
		event->operand = EVENT_NOT_EQUAL;

	} else {
		event->operand = EVENT_UNKNOWN;
	}
}

void event_set_data_type(struct event_t *event, char *data_type)
{
	// without this
	// event_set_data_type(&event, "example_string")
	//                             ^
	// was not working due to lowercase() giving seg fault
	char *data_tmp = strdup_c11(data_type);
	if (data_tmp == NULL) {
		event->type = EVENTD_UNKNOWN;
		return;
	}

	string_to_lowercase(data_tmp);

	if (strcmp(data_tmp, "decimal") == 0) {
		event->type = EVENTD_DECIMAL;

	} else if (strcmp(data_tmp, "string") == 0) {
		event->type = EVENTD_STRING;

	} else if (strcmp(data_tmp, "float") == 0) {
		event->type = EVENTD_FLOAT;

	} else {
		event->type = EVENTD_UNKNOWN;
	}

	free(data_tmp);
}

char *event_translate_operand(enum event_oper_t operand)
{
	switch (operand) {
	case EVENT_LESS:
		return "<";

	case EVENT_MORE:
		return ">";

	case EVENT_LESS_EQUAL:
		return "<=";

	case EVENT_MORE_EQUAL:
		return ">=";

	case EVENT_NOT_EQUAL:
		return "!=";

	case EVENT_EQUAL:
		return "==";

	default:
		return NULL;
	}
}

char *event_translate_data_type(enum event_data_t data_t)
{
	switch (data_t) {
	case EVENTD_DECIMAL:
		return "decimal";

	case EVENTD_STRING:
		return "string";

	default:
		return NULL;
	}
}

int event_compare_int(enum event_oper_t operand, int value, int payload_value)
{
	switch (operand) {
	case EVENT_LESS:
		return payload_value < value;

	case EVENT_MORE:
		return payload_value > value;

	case EVENT_LESS_EQUAL:
		return payload_value <= value;

	case EVENT_MORE_EQUAL:
		return payload_value >= value;

	case EVENT_NOT_EQUAL:
		return payload_value != value;

	case EVENT_EQUAL:
		return payload_value == value;

	default:
		return 0;
	}
}

int event_compare_string(enum event_oper_t operand, char *value, char *payload_value)
{
	//TODO: change strlen to strcmp
	switch (operand) {
	case EVENT_LESS:
		return strlen(payload_value) < strlen(value);

	case EVENT_MORE:
		return strlen(payload_value) > strlen(value);

	case EVENT_LESS_EQUAL:
		return strlen(payload_value) <= strlen(value);

	case EVENT_MORE_EQUAL:
		return strlen(payload_value) >= strlen(value);

	case EVENT_NOT_EQUAL:
		if (strcmp(payload_value, value) == 0) {
			return 0;
		} else
			return 1;

	case EVENT_EQUAL:
		if (strcmp(payload_value, value) == 0) {
			return 1;
		} else
			return 0;

	default:
		return 0;
	}
}

static void event_send_email(struct event_t event, char *payload_value)
{
	char subject[256];
	char body[256];

	sprintf(subject, "event triggered on %s", event.parameter);
	sprintf(body, "received value %s which is %s %s", payload_value,
		event_translate_operand(event.operand), event.value);

	event.email.subject = subject;
	event.email.body    = body;

	send_email(event.email);
}

void event_form_email(struct event_t *event, char *payload_value)
{
	char *subject = malloc(256);
	char *body    = malloc(EMAIL_BODY_SIZE);

	sprintf(subject, "event triggered on %s", event->parameter);
	sprintf(body, "received value %s which is %s %s", payload_value,
		event_translate_operand(event->operand), event->value);

	event->email.subject	   = subject;
	event->email.body	   = body;
	event->email.body_capacity = EMAIL_BODY_SIZE;
}

int event_append_body(struct event_t *event, char *msg)
{
	int msg_len	  = strlen(msg);
	int body_size	  = event->email.body_size;
	int body_capacity = event->email.body_capacity;
	char *new_string  = NULL;

	if (body_size + msg_len > body_capacity) {
		new_string		   = realloc(event->email.body, body_capacity + EMAIL_BODY_SIZE);
		event->email.body_capacity = body_capacity + EMAIL_BODY_SIZE;

		// memory reallocation failed
		if (new_string == NULL)
			return 1;

		// exchange original string with new one
		free(event->email.body);
		event->email.body = new_string;
	}

	event->email.body_size = body_size + msg_len;

	strcat(event->email.body, msg);

	return 0;
}

int event_check_trigger(struct event_t event, char *payload_value)
{
	if (event.type == EVENTD_DECIMAL) {
		// printf("value type is decimal\n");
		// possible problems if user input is not numeric
		// in that case atoi returns 0
		int value1 = atoi(event.value);
		int value2 = atoi(payload_value);
		int result = event_compare_int(event.operand, value1, value2);

		if (result == 1) {
			// triggered
			// printf("triggered\n");
			// printf("condition met, sending email\n");
			// event_send_email(event, payload_value);
			// event_form_email(&event, payload_value);
			return 1;
		}
		return 0;

	} else if (event.type == EVENTD_STRING) {
		// printf("value type is string\n");
		int result = event_compare_string(event.operand, event.value, payload_value);

		if (result == 1) {
			// triggered
			// printf("condition met, sending email\n");
			// event_send_email(event, payload_value);
			return 1;
		}
		return 0;

	} else if (event.type == EVENTD_FLOAT) {
		//check operand
	} else {
		//EVENTD_UNKNOWN
		return 0;
	}
	return 0;
}

int event_check(struct event_t *event, const struct mosquitto_message *msg, char *pl_val, int pl_val_size)
{
	char *event_topic   = event->topic;
	char *payload_topic = msg->topic;
	int ret;

	if (event->sent)
		return 0;

	if (strcmp(event_topic, payload_topic) != 0)
		return 0;

	//check event trigger
	// printf("topic matches event, checking trigger\n");

	ret = find_json_value(msg->payload, pl_val, pl_val_size, event->parameter);
	if (ret == 1) {
		// parameter not found in payload, skipping event.
		// printf("parameter not found in payload\n");
		return 0;
	}
	// printf("parameter value from payload: %s\n", pl_val);

	ret = event_check_trigger(*event, pl_val);

	// printf("email body: %s \n", event->email.body)
	if (!ret) {
		// event not triggered, skipping
		return 0;
	}

	return 1;
}

int event_compare_events(struct event_t event1, struct event_t event2)
{
	if (strcmp(event1.parameter, event2.parameter) != 0)
		return 0;

	if (strcmp(event1.email.from, event2.email.from) != 0)
		return 0;

	// Compare recipients
	for (size_t i = 0; i < event1.email.num_recipients; i++) {
		if (strcmp(event1.email.recipients[i], event2.email.recipients[i]) != 0) {
			return 0;
		}
	}
	return 1;
}

// Function to create an empty event list
struct event_list_t *event_list_create()
{
	struct event_list_t *list = malloc(sizeof(struct event_list_t));
	if (list == NULL) {
		perror("Memory allocation failed");
		return NULL;
	}
	list->events = malloc(INITIAL_CAPACITY * sizeof(struct event_t));
	if (list->events == NULL) {
		free(list);
		perror("Memory allocation failed");
		return NULL;
	}
	list->size     = 0;
	list->capacity = INITIAL_CAPACITY;
	return list;
}

// Function to add an event to the list
void event_list_add(struct event_list_t *list, const struct event_t *event)
{
	if (list == NULL || event == NULL)
		return;
	if (list->size >= list->capacity) {
		list->capacity += STEP_SIZE;
		struct event_t *temp = realloc(list->events, list->capacity * sizeof(struct event_t));
		if (temp == NULL) {
			perror("Memory allocation failed");
			return;
		}
		list->events = temp;
	}

	list->events[list->size++] = *event; // Copy the event to the list
}

void event_free(struct event_t *event)
{
	free(event->topic);
	free(event->parameter);
	free(event->value);
	email_cleanup(&event->email);
}

void event_list_free(struct event_list_t *list)
{
	if (list == NULL)
		return;
	if (list->events != NULL) {
		for (size_t i = 0; i < list->size; i++) {
			// free(list->events[i].topic);
			// free(list->events[i].parameter);
			// free(list->events[i].value);
			// email_cleanup(&list->events[i].email);
			event_free(&list->events[i]);
		}
		free(list->events);
	}
	free(list);
}

void event_list_free_without_events(struct event_list_t *list)
{
	if (list == NULL)
		return;
	if (list->events != NULL) {
		for (size_t i = 0; i < list->size; i++) {
			// free(list->events[i].topic);
			// free(list->events[i].parameter);
			// free(list->events[i].value);
			// email_cleanup(&list->events[i].email);
			// event_free(&list->events[i]);
		}
		free(list->events);
	}
	free(list);
}

void event_print(struct event_t event)
{
	printf("event topic: %s\n", event.topic);
	printf("event parameter: %s\n", event.parameter);
	printf("event type: %s\n", event_translate_data_type(event.type));
	printf("event type enum: %d\n", event.type);
	printf("event operand: %s\n", event_translate_operand(event.operand));
	printf("event operand enum: %d\n", event.operand);
	printf("event value: %s\n", event.value);
	printf("event sender: %s\n", event.email.from);
	for (int j = 0; j < event.email.num_recipients; ++j) {
		printf(" to: %s\n", event.email.recipients[j]);
	}
}

void event_list_print(struct event_list_t *event_list)
{
	for (int i = 0; i < event_list->size; ++i) {
		event_print(event_list->events[i]);
		printf("\n");
	}
}