#include <time.h>
#include <stdio.h>
#include <string.h>

#include "helper_functions.h"
#include "core_json.h"

void sig_handler(int signum)
{
	flag = 0;
}

void get_current_time(char *time_var)
{
	time_t currentTime;
	struct tm *localTime;
	char formattedTime[64];

	currentTime = time(NULL);

	localTime = localtime(&currentTime);

	strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", localTime);

	sprintf(time_var, "%s", formattedTime);
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
