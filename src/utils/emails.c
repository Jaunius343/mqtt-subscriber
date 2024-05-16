#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>

#include "emails.h"
#include "log.h"

struct upload_status {
	int lines_read;
	char **msg;
};

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
	struct upload_status *upload_ctx = (struct upload_status *)userp;
	const char **payload_text	 = (const char **)upload_ctx->msg;
	const char *data;

	if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1)) {
		return 0;
	}

	data = payload_text[upload_ctx->lines_read];

	if (data) {
		size_t len = strlen(data);
		memcpy(ptr, data, len);
		upload_ctx->lines_read++;

		return len;
	}

	return 0;
}

int send_email(struct email_t email)
{
	CURL *curl;
	CURLcode res		      = CURLE_OK;
	struct curl_slist *recipients = NULL;
	struct upload_status upload_ctx;
	char *subject = email.subject;
	char *body    = email.body;
	char *from    = email.from;

	char *payload_text[] = { "Subject: ", subject, /*user provided subject*/
				 "\r\n", /*endline for subject*/
				 "\r\n", /* empty line to divide headers from body, see RFC5322 */
				 body, /*user provided body*/
				 "\r\n", /*endline for body*/
				 NULL };

	upload_ctx.lines_read = 0;
	upload_ctx.msg	      = payload_text;

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.freesmtpservers.com");

		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from);

		log_debug("recipients num: %d", email.num_recipients);

		for (size_t i = 0; i < email.num_recipients; i++) {
			log_trace("adding recipient: %s", email.recipients[i]);
			recipients = curl_slist_append(recipients, email.recipients[i]);
		}

		// recipients = curl_slist_append(recipients, TO);
		// recipients = curl_slist_append(recipients, CC);
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

		curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
		curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		res = curl_easy_perform(curl);

		// printf("res: %d\n", res);

		if (res != CURLE_OK)
			log_error("email send failed");
		// fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

		curl_slist_free_all(recipients);

		curl_easy_cleanup(curl);

		return res;
	}
	return -1;
}

void email_init(struct email_t *email, const char *from, const char *subject, const char *body)
{
	email->from	      = strdup_c11(from);
	email->subject	      = strdup_c11(subject);
	email->body	      = strdup_c11(body);
	email->num_recipients = 0;
}

void email_add_recipient(struct email_t *email, const char *recipient)
{
	if (email->num_recipients < MAX_RECIPIENTS) {
		email->recipients[email->num_recipients] = strdup_c11(recipient);
		email->num_recipients++;
	} else {
		fprintf(stderr, "Maximum number of recipients exceeded\n");
	}
}

void email_cleanup(struct email_t *email)
{
	if (email == NULL)
		return;
	free(email->from);
	email->from = NULL;

	free(email->body);
	email->body = NULL;

	free(email->subject);
	email->subject = NULL;

	for (size_t i = 0; i < email->num_recipients; i++) {
		free(email->recipients[i]);
		email->recipients[i] = NULL;
	}

	email->num_recipients = 0;
	email->body_capacity  = 0;
	email->body_size      = 0;
}

void email_cleanup_contents(struct email_t *email)
{
	if (email == NULL)
		return;

	free(email->body);
	email->body = NULL;

	free(email->subject);
	email->subject = NULL;

	email->body_capacity = 0;
	email->body_size     = 0;
}

// strdup does not work in c11, gives segmentation fault
char *strdup_c11(const char *src)
{
	if (src == NULL) {
		return NULL;
	}

	size_t len = strlen(src) + 1;
	char *dst  = malloc(len);
	if (dst == NULL) {
		return NULL;
	}

	strcpy(dst, src);

	return dst;
}

int send_email_gmail(struct email_t email, struct smtp_settings_t *smtp)
{
	CURL *curl;
	CURLcode res		      = CURLE_OK;
	struct curl_slist *recipients = NULL;
	struct upload_status upload_ctx;
	char *subject  = email.subject;
	char *body     = email.body;
	char *from     = email.from;
	char *server   = smtp->server;
	char *user     = smtp->username;
	char *password = smtp->password;

	char *payload_text[] = { "Subject: ", subject, /*user provided subject*/
				 "\r\n", /*endline for subject*/
				 "\r\n", /* empty line to divide headers from body, see RFC5322 */
				 body, /*user provided body*/
				 "\r\n", /*endline for body*/
				 NULL };

	upload_ctx.lines_read = 0;
	upload_ctx.msg	      = payload_text;

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_USERNAME, user);
		curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
		curl_easy_setopt(curl, CURLOPT_URL, server);
		curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from);

		for (size_t i = 0; i < email.num_recipients; i++) {
			recipients = curl_slist_append(recipients, email.recipients[i]);
		}

		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

		curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
		curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
		// curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		res = curl_easy_perform(curl);

		if (res != CURLE_OK)
			log_error("email send failed: %s", curl_easy_strerror(res));

		curl_slist_free_all(recipients);

		curl_easy_cleanup(curl);

		return res;
	}

	return -1;
}