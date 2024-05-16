#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <argp.h>
#include <string.h>

#include "argp_parser.h"
#include "helper_functions.h"
#include "msg_log.h"

void on_connect(struct mosquitto *mosq, void *obj, int rc)
{
	struct arguments *arguments = (struct arguments *)obj;
	// printf("host: %s\n", arguments->host);
	// printf("topic: \'%s\'\n", arguments->topic);
	// printf("ID: %d\n", *(int *)obj);
	if (rc) {
		printf("Error with result code: %d\n", rc);
		exit(-1);
	}
	mosquitto_subscribe(mosq, NULL, "test/t1", 0);
	if (strlen(arguments->topic) != 0)
		mosquitto_subscribe(mosq, NULL, arguments->topic, 0);
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	char formattedTime[64];
	char message[128];
	get_current_time(formattedTime);
	sprintf(message, "[%s] from \"%s\": %s", formattedTime, msg->topic, (char *)msg->payload);
	printf("%s\n", message);
	msg_log(message);
}
