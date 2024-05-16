#ifndef STMP_SETTINGS_H_INCLUDED
#define STMP_SETTINGS_H_INCLUDED

struct smtp_settings_t {
	char server[256];
	char username[256];
	char password[256];
};

#endif