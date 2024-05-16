
#include <stdio.h>

#ifndef MSG_LOG_H_INCLUDED
#define MSG_LOG_H_INCLUDED

#define MSG_LOG_OK 0

extern char *file_path;

enum state_t { MSG_LOG_OPEN = 1, MSG_LOG_LOCKED, MSG_LOG_CLOSED };
enum msg_log_error_t { MSG_LOG_ERR_FILE_INIT = 1, MSG_LOG_ERR_FILE_DEINIT };

int msg_log_init();
int msg_log(char *msg);
int msg_log_deinit();
void msg_log_state();
int msg_log_get_state();

#endif