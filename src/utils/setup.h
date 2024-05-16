
#ifndef SETUP_H_INCLUDED
#define SETUP_H_INCLUDED

struct setup_t {
	int log_level;
};

struct setup_t *setup_init();
void setup_free(struct setup_t *setup);

#endif