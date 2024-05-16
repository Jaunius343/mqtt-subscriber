#include <argp.h>

#ifndef ARGP_PARSER_H_INCLUDED
#define ARGP_PARSER_H_INCLUDED

extern const char *argp_program_version;
extern const char *argp_program_bug_address;
extern char doc[];
extern char args_doc[];
extern struct argp_option options[];
extern struct argp argp;

/* Used by main to communicate with parse_opt. */
struct arguments {
	int silent, verbose, daemon;
	char output_file[128];
	char host[17];
	char topic[128];
	char user[64];
	char psw[64];
	char ca[128];
	char cl_cert[128];
	char cl_key[128];
};

error_t parse_opt(int key, char *arg, struct argp_state *state);

#endif
