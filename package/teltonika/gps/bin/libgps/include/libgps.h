#ifdef __cplusplus
extern "C" {
#endif

#pragma once

#include <libubus.h>

typedef enum {
	LGPS_SUCCESS,
	LGPS_ERROR_UBUS,
	LGPS_ERROR,
} lgps_err_t;

typedef enum {
	GPS_LONGITUDE,
	GPS_LATITUDE,
	GPS_ALTITUDE,
	GPS_ANGLE,
	GPS_SPEED,
	GPS_ACCURACY,
	GPS_SATELLITES,
	GPS_TIMESTAMP,
	GPS_STATUS,
	GPS_UTC_TIMESTAMP,
	GPS_T_MAX,
} lgps_pos_t;

typedef struct {
	double longitude;
	double latitude;
	double altitude;
	double angle;
	double speed;
	double accuracy;
	uint32_t satellites;
	uint32_t fix_status;
	uint64_t timestamp;
	uint64_t utc_timestamp;
} lgps_t;

extern const struct blobmsg_policy g_gps_position_policy[];

lgps_err_t lgps_get(struct ubus_context *, lgps_t *);
lgps_err_t lgps_subscribe(struct ubus_context *ctx, struct ubus_subscriber *gps_sub, ubus_handler_t cb);

const char *lgps_strerror(lgps_err_t);
#ifdef __cplusplus
}
#endif
