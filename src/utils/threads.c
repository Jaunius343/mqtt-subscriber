#include <stdlib.h>
#include <unistd.h>

#include "threads.h"
#include "data_container.h"
#include "log.h"

// Function to create a thread_data_t structure
struct thread_data_t *thread_data_create(pthread_mutex_t *mutex)
{
	struct thread_data_t *data = (struct thread_data_t *)malloc(sizeof(struct thread_data_t));
	if (data == NULL) {
		// Memory allocation failed
		return NULL;
	}

	// Initialize the fields of the structure
	data->reset_interval = 3600; // 1 hour in seconds, default interval
	data->mutex	     = mutex;

	return data;
}

// Function to free memory allocated for a thread_data_t structure
void thread_data_free(struct thread_data_t *data)
{
	if (data != NULL) {
		// No need to free mutex, while trying to free it's pointer
		// valgrind says mutex is allocated at stack.
		free(data);
	}
}

static void reset_flag(struct data_container_t *data_cont)
{
	struct event_list_t *event_list = data_cont->event_list;

	pthread_mutex_lock(data_cont->thread_data->mutex);

	for (int i = 0; i < event_list->size; ++i) {
		data_cont->event_list->events[i].sent = 0;
	}

	pthread_mutex_unlock(data_cont->thread_data->mutex);
}

void *thread_reset_sent_flag(void *arg)
{
	struct data_container_t *data_cont = (struct data_container_t *)arg;
	time_t start_time		   = time(NULL);

	while (flag) {
		time_t time_passed = time(NULL) - start_time;
		if (time_passed >= data_cont->thread_data->reset_interval) {
			reset_flag(data_cont);

			log_debug("------------------------------");
			log_debug("Event status reset");
			log_debug("------------------------------\n");

			start_time = time(NULL);
		}

		sleep(5);
	}

	return NULL;
}
