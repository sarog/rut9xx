
#include <libubox/blobmsg.h>
#include "lchilli.h"

enum {
	CHILLI_SESSIONS,
	__CHILLI_MAX,
};

static const struct blobmsg_policy chilli_policy[__CHILLI_MAX] = {
	[CHILLI_SESSIONS] = { .name = "sessions", .type = BLOBMSG_TYPE_ARRAY },
};

static const struct blobmsg_policy session_policy[__LCHILLI_MAX] = {
	[LCHILLI_PORT]	     = { .name = "nasPort", .type = BLOBMSG_TYPE_INT32 },
	[LCHILLI_STATE]	     = { .name = "clientState", .type = BLOBMSG_TYPE_INT32 },
	[LCHILLI_IP]	     = { .name = "ipAddress", .type = BLOBMSG_TYPE_STRING },
	[LCHILLI_URL]	     = { .name = "url", .type = BLOBMSG_TYPE_STRING },
	[LCHILLI_MAC]	     = { .name = "macAddress", .type = BLOBMSG_TYPE_STRING },
	[LCHILLI_HCPSTATE]   = { .name = "dhcpState", .type = BLOBMSG_TYPE_STRING },
	[LCHILLI_SESSION]    = { .name = "session", .type = BLOBMSG_TYPE_TABLE },
	[LCHILLI_ACCOUNTING] = { .name = "accounting", .type = BLOBMSG_TYPE_TABLE },
};

static const struct blobmsg_policy session_param_policy[__LCHILLI_SESSION_MAX] = {
	[LCHILLI_SESSION_ID]	       = { .name = "sessionId", .type = BLOBMSG_TYPE_STRING },
	[LCHILLI_SESSION_USERNAME]     = { .name = "userName", .type = BLOBMSG_TYPE_STRING },
	[LCHILLI_SESSION_START_TIME]   = { .name = "startTime", .type = BLOBMSG_TYPE_INT64 },
	[LCHILLI_SESSION_TIMEOUT]      = { .name = "sessionTimeout", .type = BLOBMSG_TYPE_INT64 },
	[LCHILLI_SESSION_TERM_TIME]    = { .name = "terminateTime", .type = BLOBMSG_TYPE_INT64 },
	[LCHILLI_SESSION_IDLE_TIMEOUT] = { .name = "idleTimeout", .type = BLOBMSG_TYPE_INT64 },
	[LCHILLI_SESSION_MAX_INPUT]    = { .name = "maxInputOctets", .type = BLOBMSG_TYPE_INT64 },
	[LCHILLI_SESSION_MAX_OUTPUT]   = { .name = "maxOutputOctets", .type = BLOBMSG_TYPE_INT64 },
	[LCHILLI_SESSION_INPUT_BW]     = { .name = "maxDwBandwidth", .type = BLOBMSG_TYPE_INT64 },
	[LCHILLI_SESSION_OUTPUT_BW]    = { .name = "maxUpBandwidth", .type = BLOBMSG_TYPE_INT64 },
};

static const struct blobmsg_policy accounting_policy[__LCHILLI_ACC_MAX] = {
	[LCHILLI_ACC_TIME]	       = { .name = "sessionTime", .type = BLOBMSG_TYPE_INT64 },
	[LCHILLI_ACC_IDLE_TIME]	       = { .name = "idleTime", .type = BLOBMSG_TYPE_INT64 },
	[LCHILLI_ACC_INPUT_OCTETS]     = { .name = "inputOctets", .type = BLOBMSG_TYPE_INT64 },
	[LCHILLI_ACC_OUTPUT_OCTETS]    = { .name = "outputOctets", .type = BLOBMSG_TYPE_INT64 },
	[LCHILLI_ACC_INPUT_GIGAWORDS]  = { .name = "inputGigawords", .type = BLOBMSG_TYPE_INT64 },
	[LCHILLI_ACC_OUTPUT_GIGAWORDS] = { .name = "outputGigawords", .type = BLOBMSG_TYPE_INT64 },
	[LCHILLI_ACC_VIEW_POINT]       = { .name = "viewPoint", .type = BLOBMSG_TYPE_STRING },
};

static struct blob_attr **lchilli_parse_blobmsg(struct blob_attr *msg, size_t len,
						struct blobmsg_policy *policy)
{
	struct blob_attr **tb;

	if (!msg) {
		return NULL;
	}

	tb = calloc(len, sizeof(struct blob_attr *));
	if (!tb) {
		return NULL;
	}

	if (blobmsg_parse(policy, len, tb, blobmsg_data(msg), blobmsg_len(msg)) != 0) {
		free(tb);

		return NULL;
	}

	return tb;
}

static void lchilli_list_callback(struct ubus_request *req, int type, struct blob_attr *msg)
{
	(void)type;
	struct blob_attr **resp = (struct blob_attr **)req->priv;
	struct blob_attr *tb[__CHILLI_MAX];

	blobmsg_parse(chilli_policy, __CHILLI_MAX, tb, blob_data(msg), blob_len(msg));
	if (tb[CHILLI_SESSIONS]) {
		*resp = blob_memdup(tb[CHILLI_SESSIONS]);
	}
}

lchilli_status lchilli_list(struct ubus_context *ubus, struct lchilli_list_req *req, struct blob_attr **msg)
{
	uint32_t id	  = 0;
	int ret		  = LCHILLI_SUCC;
	struct blob_buf b = { 0 };

	if (ubus_lookup_id(ubus, LCHILLI_UBUS_OBJECT, &id)) {
		return LCHILLI_ERR;
	}

	blob_buf_init(&b, 0);
	if (req->mac) {
		blobmsg_add_string(&b, "mac", req->mac);
	}
	if (req->session_id) {
		blobmsg_add_string(&b, "sessionid", req->session_id);
	}
	if (req->ip) {
		blobmsg_add_string(&b, "ip", req->ip);
	}

	if (ubus_invoke(ubus, id, LCHILLI_LIST_METHOD, b.head, lchilli_list_callback, msg,
			LCHILLI_UBUS_TIMEOUT * 1000)) {
		ret = LCHILLI_ERR;
	}

	blob_buf_free(&b);

	return ret;
}

struct blob_attr **lchilli_get_session(struct blob_attr *session)
{
	return lchilli_parse_blobmsg(session, __LCHILLI_MAX, session_policy);
}

struct blob_attr **lchilli_get_session_params(struct blob_attr *session)
{
	return lchilli_parse_blobmsg(session, __LCHILLI_SESSION_MAX, session_param_policy);
}

struct blob_attr **lchilli_get_accounting(struct blob_attr *session)
{
	return lchilli_parse_blobmsg(session, __LCHILLI_ACC_MAX, accounting_policy);
}
