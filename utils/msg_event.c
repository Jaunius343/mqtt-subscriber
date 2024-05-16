#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "msg_event.h"
#include "emails.h"

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

void event_check_trigger(struct event_t event, char *payload_value)
{
	if (event.type == EVENTD_DECIMAL) {
		// printf("value type is decimal\n");
		// possible problems if user input is not numeric
		// in that case atoi returns 0
		int value1 = atoi(event.value);
		int value2 = atoi(payload_value);
		int result = event_compare_int(event.operand, value1, value2);

		if (result == 1) {
			// printf("condition met, sending email\n");
			event_send_email(event, payload_value);
		}

	} else if (event.type == EVENTD_STRING) {
		// printf("value type is string\n");
		int result = event_compare_string(event.operand, event.value, payload_value);

		if (result == 1) {
			// printf("condition met, sending email\n");
			event_send_email(event, payload_value);
		}

	} else if (event.type == EVENTD_FLOAT) {
		//check operand
	} else {
		//EVENTD_UNKNOWN
	}
}