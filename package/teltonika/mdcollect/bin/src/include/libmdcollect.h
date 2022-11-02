#ifdef __cplusplus
extern "C" {
#endif
#pragma once

#include <time.h>
#include <libubus.h>
#include <libsim.h>

#define LMDC_BUFFER_SIZE 256
#define LMDC_UBUS_TIMEOUT 30
#define LMDC_UBUS_OBJECT "mdcollect"
#define LMDC_GET_DATA_METHOD "get"
#define LMDC_CLEAN_DATA_METHOD "clean_db"
#define LMDC_BACKUP_DATA_METHOD "backup_db"
#define LMDC_PERIOD_DAY "day"
#define LMDC_PERIOD_WEEK "week"
#define LMDC_PERIOD_MONTH "month"

enum { LMDC_SUCCESS, LMDC_ERROR };
enum { GET_FLAG_PERIOD, GET_FLAG_CURRENT};

typedef struct lmdc_data {
	long long tx;
	long long rx;
} lmdc_data;

/**
 * Gathers tx,rx data of the current day
 * @param ctx - ubus context
 * @param output - pointer to the output structure
 * @param sim - sim number. NULL means any
 * @param modem - modem id. NULL means any
 * @return returns LMDC_SUCCESS on success and LMDC_ERROR on fail.
 */
int lmdc_get_current_day(struct ubus_context *ctx, lmdc_data *output, lsim_t sim, const char *modem);
/**
 * Current week data
 * @param ctx - ubus context
 * @param output - pointer to the output structure
 * @param sim - sim number. NULL means any
 * @param modem - modem id. NULL means any
 * @return returns LMDC_SUCCESS on success and LMDC_ERROR on fail.
 */
int lmdc_get_current_week(struct ubus_context *ctx, lmdc_data *output, lsim_t sim, const char *modem);
/**
 * Current month
 * @param ctx - ubus context
 * @param output - pointer to the output structure
 * @param sim - sim number. NULL means any
 * @param modem - modem id. NULL means any
 * @return returns LMDC_SUCCESS on success and LMDC_ERROR on fail.
 */
int lmdc_get_current_month(struct ubus_context *ctx, lmdc_data *output, lsim_t sim, const char *modem);
/**
 * Last 24h data
 * @param ctx - ubus context
 * @param output - pointer to the output structure
 * @param sim - sim number. NULL means any
 * @param modem - modem id. NULL means any
 * @return returns LMDC_SUCCESS on success and LMDC_ERROR on fail.
 */

int lmdc_get_day(struct ubus_context *ctx, lmdc_data *output, lsim_t sim, const char *modem);
/**
 *
 * @param ctx - ubus context
 * @param output - pointer to the output structure
 * @param sim - sim number. NULL means any
 * @param modem - modem id. NULL means any
 * @return returns LMDC_SUCCESS on success and LMDC_ERROR on fail.
 */
int lmdc_get_week(struct ubus_context *ctx, lmdc_data *output, lsim_t sim, const char *modem);
/**
 *
 * @param ctx - ubus context
 * @param output - pointer to the output structure
 * @param sim - sim number. NULL means any
 * @param modem - sim number. NULL means any
 * @return returns LMDC_SUCCESS on success and LMDC_ERROR on fail.
 */
int lmdc_get_month(struct ubus_context *ctx, lmdc_data *output, lsim_t sim, const char *modem);
/**
 * Clean database
 * @param ctx - ubus context
 * @param modem - sim number. NULL means any
 * @param sim  - sim number. NULL means any
 * @param interface - interface name. NULL means any
 * @return returns LMDC_SUCCESS on success and LMDC_ERROR on fail.
 */
int lmdc_clean_db(struct ubus_context *ctx, const char *modem, lsim_t sim, const char *interface);
/**
 * Backup database
 * @param ctx - ubus context
 * @return returns LMDC_SUCCESS on success and LMDC_ERROR on fail.
 */
int lmdc_backup_db(struct ubus_context *ctx);
#ifdef __cplusplus
}
#endif
