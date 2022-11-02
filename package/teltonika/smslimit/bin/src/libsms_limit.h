#include <libubus.h>
#include <libsim.h>

typedef enum {
	LSMS_LIMIT_OK,
	LSMS_LIMIT_ERROR_UBUS,
	LSMS_LIMIT_WILL_REACH,
	LSMS_LIMIT_REACHED
} lsms_limit_t;

struct check_limit {
	int max;
	int current;
	bool reached;
};

lsms_limit_t lsms_limit_inc(struct ubus_context *ctx, lsim_t sim,
			    char *modem_id);
lsms_limit_t lsms_limit_reset(struct ubus_context *ctx, lsim_t sim,
			      char *modem_id);
lsms_limit_t lsms_limit_check(struct ubus_context *ctx, lsim_t sim,
			      char *modem_id, struct check_limit *limit);