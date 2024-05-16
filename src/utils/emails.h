// usage example

// char *subject = "test sub from main";
// 	char *body =
// 		"testing a body formatted from main \r\n \r\n new paragraph test \r\n \r\n another paragraph";
// 	struct email_t email = { 0 };

// 	email_init(&email, "from@mail.com", subject, body);
// 	printf("from email: %s \n", email.from);

// 	email_add_recipient(&email, "example@mail.com");
// 	email_add_recipient(&email, "example1@mail.com");
// 	email_add_recipient(&email, "example2@mail.com");

// 	for (size_t i = 0; i < email.num_recipients; i++) {
// 		printf("email: %s\n", email.recipients[i]);
// 	}

// 	send_email(email);

// 	email_cleanup(&email);

#include "smtp_settings.h"

#ifndef EMAILS_H_INCLUDED
#define EMAILS_H_INCLUDED

#define MAX_RECIPIENTS 10

struct email_t {
	char *from;
	char *recipients[MAX_RECIPIENTS];
	size_t num_recipients;
	char *subject;
	char *body;
	int body_size;
	int body_capacity;
};

int send_email(struct email_t email);
int send_email_gmail(struct email_t email, struct smtp_settings_t *smtp);
void email_init(struct email_t *email, const char *from, const char *subject, const char *body);
void email_add_recipient(struct email_t *email, const char *recipient);
void email_cleanup(struct email_t *email);
void email_cleanup_contents(struct email_t *email);
char *strdup_c11(const char *src);

#endif