#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "topics.h"

#define INITIAL_CAPACITY 10
#define STEP_SIZE	 10

struct topic_t *topic_create_array()
{
	struct topic_t *arr = malloc(sizeof(struct topic_t));
	if (arr == NULL) {
		// perror("Memory allocation failed");
		return NULL;
	}
	arr->strings = malloc(INITIAL_CAPACITY * sizeof(char *));
	if (arr->strings == NULL) {
		free(arr);
		// perror("Memory allocation failed");
		return NULL;
	}
	arr->size     = 0;
	arr->capacity = INITIAL_CAPACITY;
	return arr;
}

void topic_free_array(struct topic_t *arr)
{
	if (arr == NULL)
		return;
	if (arr->strings != NULL) {
		for (size_t i = 0; i < arr->size; i++) {
			free(arr->strings[i]);
		}
		free(arr->strings);
	}
	free(arr);
}

int topic_add(struct topic_t *arr, const char *str)
{
	if (arr == NULL)
		return 1;

	// Check if the string already exists
	for (int i = 0; i < arr->size; ++i) {
		if (strcmp(arr->strings[i], str) == 0) {
			// String already exists, return success
			return 0;
		}
	}

	// check if array has space, inrease if not
	if (arr->size >= arr->capacity) {
		arr->capacity += STEP_SIZE;
		char **temp = realloc(arr->strings, arr->capacity * sizeof(char *));
		if (temp == NULL) {
			// perror("Memory allocation failed");
			return 1;
		}
		arr->strings = temp;
	}
	arr->strings[arr->size++] = strdup(str);
	if (arr->strings[arr->size - 1] == NULL) {
		// perror("Memory allocation failed");
		return 1;
	}
	return 0;
}

void topic_print_array(const struct topic_t *arr)
{
	if (arr == NULL)
		return;
	printf("String Array Contents:\n");
	for (size_t i = 0; i < arr->size; i++) {
		printf("%s\n", arr->strings[i]);
	}
}