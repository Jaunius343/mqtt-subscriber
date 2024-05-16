#include <stdio.h>

#include "msg_log.h"

FILE *__msg_log_fp;
enum state_t __msg_log_file_state;
char *file_path = "mqtt_subscriber.log";
// char *file_path = "/var/log/mqtt_subscriber.log";

// TODO: remove when complete. used for debugging
void msg_log_state()
{
	printf("msg log state: %d\n", __msg_log_file_state);
}

int msg_log_get_state()
{
	return __msg_log_file_state;
}

int msg_log_init()
{
	__msg_log_fp = fopen(file_path, "a");
	if (__msg_log_fp == NULL) {
		return MSG_LOG_ERR_FILE_INIT;
	}
	__msg_log_file_state = MSG_LOG_OPEN;
	return MSG_LOG_OK;
}

int msg_log(char *msg)
{
	if (__msg_log_file_state != MSG_LOG_LOCKED) {
		__msg_log_file_state = MSG_LOG_LOCKED;
		fprintf(__msg_log_fp, "%s\n", msg);
		__msg_log_file_state = MSG_LOG_OPEN;
	}
	return MSG_LOG_OK;
}

int msg_log_deinit()
{
	int rtn;
	if (__msg_log_file_state == MSG_LOG_OPEN) {
		rtn = fclose(__msg_log_fp);

		if (rtn != 0) {
			return MSG_LOG_ERR_FILE_DEINIT;
		}

		__msg_log_file_state = MSG_LOG_CLOSED;
	}
	return MSG_LOG_OK;
}