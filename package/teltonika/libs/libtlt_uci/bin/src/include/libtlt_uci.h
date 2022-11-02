/**
 * Copyright 
 *
 * @file libtlt_uci.h
 * @brief Uci library wrapper for abscrated configuration managment
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LIB_TLT_UCI
#define LIB_TLT_UCI
#include <uci.h>
#include <stdlib.h>
#include <string.h>

/**** flags for uci ****/
static enum {
	CLI_FLAG_MERGE	  = (1 << 0),
	CLI_FLAG_QUIET	  = (1 << 1),
	CLI_FLAG_NOCOMMIT = (1 << 2),
	CLI_FLAG_BATCH	  = (1 << 3),
	CLI_FLAG_SHOW_EXT = (1 << 4),
} flags;

/**
 * @brief Add uci section or uci option
 * @param ctx uci context reference
 * @param p configuration name
 * @param s section name
 * @param o option name
 * @param t option value
 * @return return 0 on success or error code
 */
int ucix_add_option(struct uci_context *ctx, const char *p, const char *s, const char *o, const char *t);

/**
 * @brief get option value
 * @param ctx uci context reference
 * @param p configuration name
 * @param s section name
 * @param o option name
 */
char *ucix_get_option(struct uci_context *ctx, const char *p, const char *s, const char *o);

/**
 * @brief get option value and convert to integer type
 * @param ctx uci context reference
 * @param p configuration name
 * @param s section name
 * @param o option name
 * @param def default value on failure
 * @return convert option value
 */
int ucix_get_option_int(struct uci_context *ctx, const char *p, const char *s, const char *o, int def);

/**
 * Same as *ucix_get_option* function
 * @brief get option value
 * @param ctx uci context reference
 * @param p configuration name
 * @param s section name
 * @param o option name
 */
char *ucix_get_option_cfg(struct uci_context *ctx, const char *p, const char *s, const char *o);

/**
 * @brief add or set option value
 * @param ctx uci context reference
 * @param p configuration name
 * @param s section name
 * @param o option name
 * @param t option value
 * @return return 0 on success or error code
 */
int ucix_add_option_int(struct uci_context *ctx, const char *p, const char *s, const char *o, int t);

/**
 * @brief import uci configuration from file to uci context
 * @param ctx uci context
 * @param name uci config name
 * @param path file path
 * @return return 0 on success
 */
int ucix_do_import(struct uci_context *ctx, const char *name, const char *path);

/**
 * @brief commit and save uci changes
 * @param ctx uci context
 * @param p configuration name
 * @return on success function return 0, on failure -1 and errno is set
 */
int ucix_commit(struct uci_context *ctx, const char *p);

/**
 * @brief free uci context
 * @param ctx uci context
 */
void uci_cleanup(struct uci_context *ctx);

/**
 * @brief allocate memory for uci context and initialize objects
 * @return allocated uci context
 */
struct uci_context *uci_init();

/**
 * @brief print error message
 */
void cli_perror(struct uci_context *ctx, char *str);

/**
 * @brief get list
 * @param ctx uci context
 * @param p configuration name
 * @param s section name
 * @param o option name
 * @return char string of value. On failure returns NULL
 */
char *ucix_get_list_option(struct uci_context *ctx, const char *p, const char *s, const char *o);

/**
 * @brief count sections with the same type
 * @param pkg uci package
 * @param t section type
 * @return number of sections found
 */
size_t ucix_count_sections(struct uci_package *pkg, const char *t);

#endif

#ifdef __cplusplus
}
#endif
