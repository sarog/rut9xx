#include <libubus.h>

typedef enum {
	LRUT_FOTA_OK,
	LRUT_FOTA_UBUS_ERR
} lrut_fota_t;

struct lrut_fota_process_st {
	int percents;
	char *process;
};

struct lrut_fota_info_st {
	char *fw;
	int fw_size;
	char *modem;
	int modem_size;
	char *conn_state;
};

lrut_fota_t lrut_fota_get_process(struct ubus_context *ubus, struct lrut_fota_process_st *process);
lrut_fota_t lrut_fota_set_process(struct ubus_context *ubus, int percents, char *process);
lrut_fota_t lrut_fota_get_info(struct ubus_context *ubus, struct lrut_fota_info_st *info);
lrut_fota_t lrut_fota_set_info(struct ubus_context *ubus, int fw_size, char *fw, int modem_size, char *modem, char *conn_state);
