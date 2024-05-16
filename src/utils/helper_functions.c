#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "helper_functions.h"
#include "core_json.h"
#include "log.h"

void sig_handler(int signum)
{
	flag = 0;
}

// void get_current_time(char *time_var)
// {
// 	time_t currentTime;
// 	struct tm *localTime;
// 	char formattedTime[64];

// 	currentTime = time(NULL);

// 	localTime = localtime(&currentTime);

// 	strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", localTime);

// 	sprintf(time_var, "%s", formattedTime);
// }

void log_init_message(FILE *fp)
{
	if (fp == NULL)
		return;

	fprintf(fp, "-------------------------------------------------\n");
	fprintf(fp, "\n");
	fprintf(fp, "                   starting new session\n");
	fprintf(fp, "\n");
	fprintf(fp, "-------------------------------------------------\n");
}

int find_json_value(char *string, char *dest, size_t dest_s, char *parameter)
{
	JSONStatus_t result;
	int len = strlen(string);
	char buffer[len];
	size_t bufferLength   = sizeof(buffer) - 1;
	char *queryKey	      = parameter;
	size_t queryKeyLength = strlen(queryKey);
	char *value;
	size_t valueLength;

	strcpy(buffer, string);

	result = JSON_Search(buffer, bufferLength, queryKey, queryKeyLength, &value, &valueLength);

	if (result == JSONSuccess) {
		// The pointer "value" will point to a location in the "buffer".
		char save = value[valueLength];
		// After saving the character, set it to a null byte for printing.
		value[valueLength] = '\0';

		strncpy(dest, value, dest_s - 1);

		// Restore the original character.
		value[valueLength] = save;

		return 0;
	}
	return 1;
}

FILE *file_cb_init(int log_level)
{
	FILE *file;
	struct stat st	   = { 0 };
	char *file_sub_dir = "logs";
	char *file_name	   = "mqtt_sub.log";
	char file_dir[128];
	char file_path[256];

	char *home_dir = getenv("HOME");
	if (home_dir == NULL) {
		log_warn("HOME environment variable is not set");
		return NULL;
	}

	// create full directory
	sprintf(file_dir, "%s/%s", home_dir, file_sub_dir);

	// check if directory exists
	if (stat(file_dir, &st) == -1) {
		// create dir if it doesn't exist
		mkdir(file_dir, 0777);
	}

	// create full path to file
	sprintf(file_path, "%s/%s", file_dir, file_name);

	file = fopen(file_path, "a");
	if (file == NULL) {
		log_info("couldn't open log folder");
		return NULL;
	}

	log_add_fp(file, log_level);
	log_init_message(file);

	log_info("logging messages to: \"%s\"", file_path);

	return file;
}
