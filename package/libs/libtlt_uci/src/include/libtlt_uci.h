#ifndef LIB_TLT_UCI
#define LIB_TLT_UCI
#include <uci.h>
#include <stdlib.h>
#include <string.h>

//=== uci functions
void ucix_add_option(struct uci_context *ctx, const char *p, const char *s, const char *o, const char *t);
void ucix_add_option_cfg(struct uci_context *ctx, const char *p, const char *s, const char *o, const char *t);
char *ucix_get_option(struct uci_context *ctx, const char *p, const char *s, const char *o);
char *ucix_get_option_cfg(struct uci_context *ctx, const char *p, const char *s, const char *o);
int ucix_commit(struct uci_context *ctx, const char *p);
void uci_cleanup(struct uci_context *ctx);
struct uci_context* uci_init();
#endif
