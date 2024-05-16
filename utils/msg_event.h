// example usage
// struct event_t event = { 0 };
// event.topic	     = "test/t1";
// event.parameter	     = "data.temp";
// event.value	     = "20";
// event_set_operand(&event, "<=");
// event_set_data_type(&event, "decimal");
// event.email = email;

// char *payload =
// 	" {\"name\":  \"outdoor thermometer\", \"id\": \"thermo_123\", \"data\": {\"temp\": \"20\", \"humidity\": 60}}";

// char payload_value[128];
// int ret;
// ret = find_json_value(payload, payload_value, sizeof(payload_value), event.parameter);
// if (ret == 1) {
// 	printf("parameter not found");
// }
// printf("result: %s\n", payload_value);

// event_check_trigger(event, payload_value);

#include "emails.h"

#ifndef MSG_EVENT_H_INCLUDED
#define MSG_EVENT_H_INCLUDED

enum event_oper_t {
	EVENT_UNKNOWN,
	EVENT_LESS,
	EVENT_MORE,
	EVENT_EQUAL,
	EVENT_NOT_EQUAL,
	EVENT_MORE_EQUAL,
	EVENT_LESS_EQUAL
};

enum event_data_t { EVENTD_UNKNOWN, EVENTD_DECIMAL, EVENTD_STRING, EVENTD_FLOAT };

struct event_t {
	char *topic;
	char *parameter;
	enum event_data_t type;
	enum event_oper_t operand;
	char *value;
	struct email_t email;
};

void event_set_operand(struct event_t *event, char *operand);
void event_set_data_type(struct event_t *event, char *data_type);
char *event_translate_operand(enum event_oper_t operand);
int event_compare_int(enum event_oper_t operand, int value, int payload_value);
int event_compare_string(enum event_oper_t operand, char *value, char *payload_value);
void event_check_trigger(struct event_t event, char *payload_value);

#endif