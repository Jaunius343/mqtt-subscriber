#ifndef TOPICS_H_INCLUDED
#define TOPICS_H_INCLUDED

struct topic_t {
	char **strings;
	size_t size;
	size_t capacity;
};

struct topic_t *topic_create_array();
void topic_free_array(struct topic_t *arr);
int topic_add(struct topic_t *arr, const char *str);
void topic_print_array(const struct topic_t *arr);

#endif