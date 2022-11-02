#ifndef __EVENT_H
#define __EVENT_H

#include "mdcollectd.h"

#define UBUS_METHOD_NET "network.interface"

enum {
	IFACE_UP,
	IFACE_DEVICE,
	IFACE_L3DEVICE,
    IFACE_NAME,
	IFACE_DATA,
	__IFACE_MAX
};

enum {
	IFACE_DATA_MODEM,
	IFACE_DATA_SIM,
    IFACE_DATA_PDH4,
    IFACE_DATA_PDH6,
	__IFACE_DATA_MAX
};

extern const struct blobmsg_policy iface_policy[__IFACE_MAX];
extern const struct blobmsg_policy iface_data_policy[__IFACE_DATA_MAX];

md_status_t subscriber_event_register(struct ubus_context *ctx);

#endif //__EVENT_H
