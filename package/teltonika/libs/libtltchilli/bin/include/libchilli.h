#ifndef __LCHILLI_H
#define __LCHILLI_H

#include <libubus.h>
#include <libubox/blobmsg.h>

#define LCHILLI_UBUS_OBJECT "chilli"
#define LCHILLI_LIST_METHOD "list"

#define LCHILLI_UBUS_TIMEOUT 30

enum {
	LCHILLI_PORT,
	LCHILLI_STATE,
	LCHILLI_IP,
	LCHILLI_URL,
	LCHILLI_MAC,
	LCHILLI_HCPSTATE,
	LCHILLI_SESSION,
	LCHILLI_ACCOUNTING,
	__LCHILLI_MAX,
};

enum {
	LCHILLI_SESSION_ID,
	LCHILLI_SESSION_USERNAME,
	LCHILLI_SESSION_START_TIME,
	LCHILLI_SESSION_TIMEOUT,
	LCHILLI_SESSION_TERM_TIME,
	LCHILLI_SESSION_IDLE_TIMEOUT,
	LCHILLI_SESSION_MAX_INPUT,
	LCHILLI_SESSION_MAX_OUTPUT,
	LCHILLI_SESSION_INPUT_BW,
	LCHILLI_SESSION_OUTPUT_BW,
	__LCHILLI_SESSION_MAX,
};

enum {
	LCHILLI_ACC_TIME,
	LCHILLI_ACC_IDLE_TIME,
	LCHILLI_ACC_INPUT_OCTETS,
	LCHILLI_ACC_OUTPUT_OCTETS,
	LCHILLI_ACC_INPUT_GIGAWORDS,
	LCHILLI_ACC_OUTPUT_GIGAWORDS,
	LCHILLI_ACC_VIEW_POINT,
	__LCHILLI_ACC_MAX,
};

typedef enum { LCHILLI_SUCC, LCHILLI_ERR } lchilli_status;

struct lchilli_list_req {
	char *mac;
	char *session_id;
	char *ip;
};

lchilli_status lchilli_list(struct ubus_context *ubus, struct lchilli_list_req *req, struct blob_attr **msg);

struct blob_attr **lchilli_get_session(struct blob_attr *session);

struct blob_attr **lchilli_get_session_params(struct blob_attr *session);

#define LCHILLI_SESSION_COUNT(x) blobmsg_check_array(x, BLOBMSG_TYPE_TABLE)

#endif /* __LCHILLI_H */
