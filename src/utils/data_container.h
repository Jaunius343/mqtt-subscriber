#include "msg_event.h"
#include "topics.h"
#include "argp_parser.h"
#include "threads.h"
#include "smtp_settings.h"
#include "setup.h"

#ifndef DATA_CONTAINER_H_INCLUDED
#define DATA_CONTAINER_H_INCLUDED

struct data_container_t {
	struct event_list_t *event_list;
	struct topic_t *topics;
	struct arguments *args;
	struct thread_data_t *thread_data;
	struct smtp_settings_t *smtp;
	struct setup_t *setup;
};

#endif