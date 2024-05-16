#include <uci.h>
#include <string.h>
#include <stdlib.h>

#include "emails.h"
#include "log.h"
#include "data_container.h"

void uci_read_option_list(struct uci_option *option, char *section_type, struct topic_t *topics,
			  struct email_t *email)
{
	struct uci_element *elem_list;

	uci_foreach_element (&option->v.list, elem_list) {
		if (strcmp(section_type, "topics") == 0) {
			topic_add(topics, elem_list->name);
		}

		if (strcmp(section_type, "event") == 0) {
			email_add_recipient(email, elem_list->name);
		}
	}
}

void uci_assign_event_vars(struct uci_option *option, struct event_t *event)
{
	char *option_name = option->e.name;

	if (strcmp(option_name, "topic") == 0) {
		event->topic = strdup(option->v.string);

	} else if (strcmp(option_name, "parameter") == 0) {
		event->parameter = strdup(option->v.string);

	} else if (strcmp(option_name, "value") == 0) {
		event->value = strdup(option->v.string);

	} else if (strcmp(option_name, "operand") == 0) {
		event_set_operand(event, option->v.string);

	} else if (strcmp(option_name, "type") == 0) {
		event_set_data_type(event, option->v.string);

	} else if (strcmp(option_name, "sender") == 0) {
		event->email.from = strdup(option->v.string);
	}
}

void uci_assign_smtp_vars(struct uci_option *option, struct smtp_settings_t *smtp)
{
	char *option_name = option->e.name;

	if (strcmp(option_name, "server") == 0) {
		strncpy(smtp->server, option->v.string, sizeof(smtp->server));

	} else if (strcmp(option_name, "username") == 0) {
		strncpy(smtp->username, option->v.string, sizeof(smtp->username));

	} else if (strcmp(option_name, "password") == 0) {
		strncpy(smtp->password, option->v.string, sizeof(smtp->password));
	}
}

void uci_assign_setup_vars(struct uci_option *option, struct data_container_t *data_cont)
{
	char *option_name		  = option->e.name;
	struct thread_data_t *thread_data = data_cont->thread_data;

	if (strcmp(option_name, "smtp_reset_interval") == 0) {
		thread_data->reset_interval = atoi(option->v.string);

	} else if (strcmp(option_name, "log_level") == 0) {
		int level = atoi(option->v.string);
		if (level >= 0 && level <= 5)
			data_cont->setup->log_level = level;
		else
			level = 2;
	}
}

void uci_read_option_string(struct uci_option *option, char *section_type, struct event_t *event,
			    struct data_container_t *data_cont)
{
	struct smtp_settings_t *smtp = data_cont->smtp;

	if (strcmp(section_type, "event") == 0) {
		uci_assign_event_vars(option, event);

	} else if (strcmp(section_type, "smtp") == 0) {
		uci_assign_smtp_vars(option, smtp);

	} else if (strcmp(section_type, "setup") == 0) {
		uci_assign_setup_vars(option, data_cont);
	}
}

void uci_read_options(struct uci_section *section, struct event_t *event, struct data_container_t *data_cont)
{
	struct uci_element *j;
	char *section_type     = section->type;
	struct topic_t *topics = data_cont->topics;

	uci_foreach_element (&section->options, j) {
		struct uci_option *option = uci_to_option(j);

		if (option->type == UCI_TYPE_STRING) {
			uci_read_option_string(option, section_type, event, data_cont);

		} else {
			uci_read_option_list(option, section_type, topics, &event->email);
		}
	}
}

int uci_event_is_valid(struct event_t event)
{
	if (event.topic == NULL || event.parameter == NULL || event.type == EVENTD_UNKNOWN ||
	    event.operand == EVENT_UNKNOWN || event.value == NULL || event.email.from == NULL ||
	    event.email.num_recipients == 0)
		return 0;
	else
		return 1;
}

void uci_read_file(struct data_container_t *data_cont)
{
	const char *config_name = "mqtt_sub";

	struct uci_context *context = uci_alloc_context();
	struct uci_package *package;

	struct event_list_t *event_list = data_cont->event_list;

	// It is also possible to load the config with uci_lookup_ptr(),
	// or from a file stream using uci_import()
	if (uci_load(context, config_name, &package) != UCI_OK) {
		uci_perror(context, "uci_load()");
		uci_free_context(context);
		return;
	}

	struct uci_element *i;
	uci_foreach_element (&package->sections, i) {
		struct uci_section *section = uci_to_section(i);
		struct event_t event	    = { 0 };
		email_init(&event.email, NULL, NULL, NULL);

		uci_read_options(section, &event, data_cont);

		// add event to list
		if (strcmp(section->type, "event") == 0) {
			if (uci_event_is_valid(event)) {
				event_list_add(event_list, &event);

			} else {
				log_warn("found an incomplete event, skipping it\n");
				event_free(&event);
			}
		}
	}

	uci_free_context(context);
}
