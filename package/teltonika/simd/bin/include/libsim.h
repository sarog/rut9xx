#pragma once

#include <libubus.h>

typedef enum {
	// modifications other than adding additional SIMs require code modifications
	LSIM_UNKNOWN,
	LSIM_SWAP = LSIM_UNKNOWN,
	LSIM_ONE,
	LSIM_TWO,
	LSIM_MAX,
} lsim_t;

typedef enum {
	LSIM_OK,
	LSIM_ERROR_UBUS,
	LSIM_ERROR_FILE,
	LSIM_ERROR_JSON,
	LSIM_ERROR_SIM_OUT_OF_RANGE,
} lsim_err_t;

lsim_err_t lsim_get(struct ubus_context *, lsim_t *);
lsim_err_t lsim_get_count(bool *single_sim, char *modem_id);
int lsim_count( char *modem_id, int *count);
int lsim_sim_count_find( char *modem_id, int *count);
lsim_err_t lsim_change(struct ubus_context *, lsim_t);
const char *lsim_strerror(lsim_err_t);
