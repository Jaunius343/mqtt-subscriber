#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <signal.h>
#include <argp.h>
#include <string.h>
#include <curl/curl.h>

#include "argp_parser.h"
#include "helper_functions.h"
#include "mqtt_func.h"

#include "msg_log.h"
#include "emails.h"
#include "msg_event.h"

int flag = 1;

int main(int argc, char **argv)
{
	struct email_t email = { 0 };
	email_init(&email, "from@mail.com", NULL, NULL);
	email_add_recipient(&email, "example@mail.com");

	// send_email(email);

	// email_cleanup(&email);

	// printf("atoi: %d\n", atoi("test0"));
	// int result = event_compare_int(EVENT_MORE_EQUAL, 2, 2);
	// printf("result: %d\n", result);

	struct event_t event = { 0 };
	event.topic	     = "test/t1";
	event.parameter	     = "data.temp";
	event.value	     = "20";
	event_set_operand(&event, "<=");
	event_set_data_type(&event, "decimal");
	event.email = email;

	// email_add_recipient(&event.email, "example2@mail.com");
	// printf("email num: %d\n", event.email.num_recipients);

	// printf("email test: %s\n", event.email.from);

	// printf("email oper: %d\n", event.operand);
	// printf("data type: %d\n", event.type);

	// decimal - "data": {"temp": 20}
	// string - "data": {"temp": "20"}
	char *payload =
		" {\"name\":  \"outdoor thermometer\", \"id\": \"thermo_123\", \"data\": {\"temp\": \"20\", \"humidity\": 60}}";

	char payload_value[128];
	int ret;
	ret = find_json_value(payload, payload_value, sizeof(payload_value), event.parameter);
	if (ret == 1) {
		printf("parameter not found");
	}
	printf("result: %s\n", payload_value);

	event_check_trigger(event, payload_value);

	return 0;

	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);

	struct arguments args	 = { 0 };
	struct arguments *args_t = &args;

	int rc, id = 12;
	struct mosquitto *mosq;

	rc = msg_log_init();
	if (rc != MSG_LOG_OK) {
		flag = 0;
	}

	argp_parse(&argp, argc, argv, 0, 0, args_t);

	// printf("arguments ca: %s\n", arguments.ca);

	mosquitto_lib_init();

	// mosq = mosquitto_new("subscribe-test", true, &id);
	mosq = mosquitto_new("subscribe-test", true, &args_t);

	// check if user is trying to connect with username and password
	if (strlen(args.user) != 0 || strlen(args.psw) != 0) {
		if (strlen(args.user) == 0) {
			printf("-P option requires -u\n");

		} else if (strlen(args.psw) == 0) {
			printf("-u option requires -P\n");

		} else {
			mosquitto_username_pw_set(mosq, args.user, args.psw);
		}
	}

	if (*args.ca != 0 && *args.cl_cert != 0 && *args.cl_key != 0) {
		printf("TLS set\n");
		mosquitto_tls_set(mosq, args.ca, NULL, args.cl_cert, args.cl_key, NULL);

	} else if (*args.ca != 0 || *args.cl_cert != 0 || args.cl_key != 0) {
		printf("TLS usage: \n");
		printf("mqtt_sub [OPTIONS] --cafile <path> --cert <path> --key <path>\n");
	}

	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);

	rc = mosquitto_connect(mosq, args.host, 1883, 10);
	printf("mosq connect status: %d\n", rc);
	if (rc) {
		printf("Could not connect to Broker with return code %d\n", rc);
		// return -1;
	}

	if (rc == MOSQ_ERR_SUCCESS)
		mosquitto_loop_start(mosq);

	while (flag) {
		;
	}

	mosquitto_loop_stop(mosq, true);

	mosquitto_disconnect(mosq);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();

	msg_log_deinit();

	return 0;
}
