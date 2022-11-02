#pragma once

#include "system.h"

#define LMDC_UBUS_SLEEP_TIME 5 // seconds

#define LMDC_PERIOD_DAY	  "day"
#define LMDC_PERIOD_WEEK  "week"
#define LMDC_PERIOD_MONTH "month"

enum md_ubus_ret { MD_UBUS_SUCCESS, MD_UBUS_ERROR };

md_status_t md_ubus_init(struct ubus_context *ubus_ctx);
