#include <argp.h>
#include <string.h>

#include "argp_parser.h"

const char *argp_program_version     = "mqtt_sub";
const char *argp_program_bug_address = "<jaunius.tamulevicius@teltonika.lt>";

/* Program documentation. */
char doc[] = "Options: ";

/* A description of the arguments we accept. */
char args_doc[] = "";

enum option_group_t { DEFAULT_GROUP = 0, MAIN_GROUP, USER_GROUP, TLS_GROUP, EXTRAS_GROUP };

/* The options we understand. */
struct argp_option options[] = { { 0, 0, 0, 0, "Extras", EXTRAS_GROUP },
				 { "verbose", 'v', 0, 0, "Produce verbose output", EXTRAS_GROUP },
				 { "quiet", 'q', 0, 0, "Don't produce any output", EXTRAS_GROUP },
				 { "silent", 's', 0, OPTION_ALIAS },
				 { "output", 'o', "<FILE>", 0, "Output to FILE instead of standard output",
				   EXTRAS_GROUP },
				 { "daemon", 'd', 0, 0, "launch as daemon", EXTRAS_GROUP },

				 { 0, 0, 0, 0, "main commands", MAIN_GROUP },
				 { "host", 'h', "<arg>", 0, "broker host ip address" },
				 { "topic", 't', "<arg>", 0, "subscription topic" },

				 { 0, 0, 0, 0, "TLS commands", TLS_GROUP },
				 { "cafile", 'c', "<path>", 0, "ca file" },
				 { "cert", 'e', "<path>", 0, "client cert file" },
				 { "key", 'k', "<path>", 0, "client key file" },

				 { 0, 0, 0, 0, "User login group", USER_GROUP },
				 { "user", 'u', "<arg>", 0, "login user", USER_GROUP },
				 { "password", 'P', "<arg>", 0, "login password", USER_GROUP },
				 { 0 } };

struct argp argp = { options, parse_opt, args_doc, doc };

/* Parse a single option. */
error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	/* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
	struct arguments *arguments = state->input;

	switch (key) {
	case 'q':
	case 's':
		arguments->silent = 1;
		break;

	case 'v':
		arguments->verbose = 1;
		break;

	case 'o':
		strncpy(arguments->output_file, arg, sizeof(arguments->output_file) - 1);
		break;

	case 'd':
		arguments->daemon = 1;
		break;

	case 'h':
		strncpy(arguments->host, arg, sizeof(arguments->host) - 1);
		break;

	case 't':
		strncpy(arguments->topic, arg, sizeof(arguments->topic) - 1);
		break;

	case 'u':
		strncpy(arguments->user, arg, sizeof(arguments->user) - 1);
		break;

	case 'P':
		strncpy(arguments->psw, arg, sizeof(arguments->psw) - 1);
		break;

	case 'c':
		strncpy(arguments->ca, arg, sizeof(arguments->ca) - 1);
		break;

	case 'e':
		strncpy(arguments->cl_cert, arg, sizeof(arguments->cl_cert) - 1);
		break;

	case 'k':
		strncpy(arguments->cl_key, arg, sizeof(arguments->cl_key) - 1);
		break;

	// override default "too many arguments" handling.
	// Without this passing positional arguments gets memory allocated
	// and argp exits the program with memory leak.
	case ARGP_KEY_ARG:
		if (state->arg_num >= 0)
			;
		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

// Custom parser for long options without equals sign
// like "--cafile test" in addition to --cafile=test
// to resemble mosquitto_sub
static error_t parse_long_opt(int key, char *arg, struct argp_state *state)
{
	if (arg) {
		return parse_opt(key, arg, state);
	} else {
		return ARGP_ERR_UNKNOWN;
	}
}
