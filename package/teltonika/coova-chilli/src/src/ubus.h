
#ifndef _CHILLI_UBUS_H
#define _CHILLI_UBUS_H

#include <libubus.h>

#define CHILLI_UBUS_OBJ_BUFSIZ 50

// various defines for parsing the incoming ubus obj
#define CHILLI_UBUS_HAP_OBJ "hostapd"
#define CHILLI_UBUS_HAP_DISASSOC "disassoc"
#define CHILLI_UBUS_HAP_ADDR "address"

#define UNUSED(x) (void)(x)

void chilli_ubus_add_obj(struct ubus_context *ctx);

void chilli_ubus_subscribe_hostapd(struct ubus_context *ctx, struct options_t options);

#endif //_CHILLI_UBUS_H
