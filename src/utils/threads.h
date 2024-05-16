#include <pthread.h>

#ifndef THREADS_H_INCLUDED
#define THREADS_H_INCLUDED

extern int flag;

struct thread_data_t {
	int reset_interval;
	pthread_mutex_t *mutex;
};

struct thread_data_t *thread_data_create(pthread_mutex_t *mutex);
void thread_data_free(struct thread_data_t *data);
void *thread_reset_sent_flag(void *arg);

#endif