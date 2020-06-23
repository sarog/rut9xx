#ifndef UCI_FUNCTION_H
#define UCI_FUNCTION_H

#include <uci.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
	CONFIG,
	SECTION,
	OPTION,
	VALUE
} uci_action;

void ucix_add_option(struct uci_context *ctx, const char *p, const char *s, const char *o, const char *t);
void ucix_add_option_cfg(struct uci_context *ctx, const char *p, const char *s, const char *o, const char *t);
char* ucix_get_option(struct uci_context *ctx, const char *p, const char *s, const char *o);
char* ucix_get_option_cfg(struct uci_context *ctx, const char *p, const char *s, const char *o);

/**
 * Converts extended uci section into a section that you can compare against iterations.
 *
 * Upon succesful execution it will change 'section' to cfg<id> and will return UCI_OK.
 * Otherwise it will return UCI_ERR_NOTFOUND, or -1 if memory allocation fails.
 */
int ucix_convert_extended(struct uci_context *ctx, const char *package, char **section);

/*
 * Essentially does the same as 'uci show <args>'
 *
 * Upon succesful execution will return a pointer to a string containing the uci
 * configuration text.
 *
 * Otherwise, upon failure or if it could not find the specified configuration will
 * return NULL.
 */
char *ucix_show_cfg(struct uci_context *ctx, uci_action do_action, const char *pac,
	const char *sec, const char *opt);

/**
 * Deletes a uci package, section or option.
 *
 * Upon succesful execution will return 0
 * Otherwise if it cannot find the section it will return 1
 */
int ucix_delete(struct uci_context *ctx, const char *p, const char *s, const char *o);

/*
 * Saves specified configuration (package) to flash.
 *
 * Upon succesful execution will return 0.
 */
int ucix_commit(struct uci_context *ctx, const char *p);

void uci_cleanup(struct uci_context *ctx);

#endif
