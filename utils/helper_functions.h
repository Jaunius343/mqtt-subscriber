#ifndef HELPER_FUNCTIONS_H_INCLUDED
#define HELPER_FUNCTIONS_H_INCLUDED

extern int flag;
void sig_handler(int signum);
void get_current_time(char *time_var);
int find_json_value(char *string, char *dest, size_t dest_s, char *parameter);

#endif