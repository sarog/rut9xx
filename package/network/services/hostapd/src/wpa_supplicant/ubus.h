/*
 * wpa_supplicant / ubus support
 * Copyright (c) 2018, Daniel Golle <daniel@makrotopia.org>
 * Copyright (c) 2013, Felix Fietkau <nbd@nbd.name>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */
#ifndef __WPAS_UBUS_H
#define __WPAS_UBUS_H

struct wpa_supplicant;
struct wpa_global;

#include "wps_supplicant.h"

#ifdef UBUS_SUPPORT
#include <libubus.h>

enum wpas_ubus_event_type {
	WPAS_UBUS_PROBE_REQ,
	WPAS_UBUS_BEACON,
	WPAS_UBUS_TYPE_MAX
};

struct wpas_ubus_bss {
	struct ubus_object obj;
};

struct wpas_frame_info {
        unsigned int freq;
        u32 channel;
        u32 datarate;
        int ssi_signal; /* dBm */
};

struct wpas_ubus_request {
	enum wpas_ubus_event_type type;
	const struct ieee80211_mgmt *mgmt_frame;
	const struct ieee802_11_elems *elems;
	const struct wpas_frame_info *frame_info;
	const u8 *addr;
};

void wpas_ubus_add_bss(struct wpa_supplicant *wpa_s);
void wpas_ubus_free_bss(struct wpa_supplicant *wpa_s);

void wpas_ubus_add(struct wpa_global *global);
void wpas_ubus_free(struct wpa_global *global);

#ifdef CONFIG_WPS
void wpas_ubus_notify(struct wpa_supplicant *wpa_s, const struct wps_credential *cred);
#endif

int wpas_ubus_handle_event(struct wpa_supplicant *wpa_s, struct wpas_ubus_request *req);

#else
struct wpas_ubus_bss {};

static inline void wpas_ubus_add_iface(struct wpa_supplicant *wpa_s)
{
}

static inline void wpas_ubus_free_iface(struct wpa_supplicant *wpa_s)
{
}

static inline void wpas_ubus_add_bss(struct wpa_supplicant *wpa_s)
{
}

static inline void wpas_ubus_free_bss(struct wpa_supplicant *wpa_s)
{
}

static inline void wpas_ubus_notify(struct wpa_supplicant *wpa_s, struct wps_credential *cred)
{
}

static inline void wpas_ubus_add(struct wpa_global *global)
{
}

static inline void wpas_ubus_free(struct wpa_global *global)
{
}
#endif

#endif
