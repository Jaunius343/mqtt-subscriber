#include <stdlib.h>

#include "setup.h"
#include "log.h"

struct setup_t *setup_init()
{
	struct setup_t *setup = (struct setup_t *)malloc(sizeof(struct setup_t));
	if (setup == NULL) {
		// Memory allocation failed
		return NULL;
	}

	setup->log_level = LOG_H_INFO;

	return setup;
}

void setup_free(struct setup_t *setup)
{
	if (setup != NULL) {
		free(setup);
	}
}