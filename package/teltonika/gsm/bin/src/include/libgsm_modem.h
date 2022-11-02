#ifdef __cplusplus
extern "C" {
#endif

#pragma once

#include <json-c/json.h>
#include <libubus.h>
#include <libubox/blobmsg_json.h>
#include <libubox/blobmsg.h>
#include <libubox/blob.h>

#define UBUS_R2EC_OBJECT "r2ec"
#define UBUS_SET_GPIO_METHOD "gpio.set"
#define UBUS_RESET_GPIO_METHOD "gpio.reset"
#define PIN "pin"
#define ID "id"
#define MODEM_REBOOT_PIN "lte_rst"

#define VID_PID "vidpid"
#define UBUS_GSMD_OBJECT "gsmd"
#define UBUS_FILE_OBJECT "file"
#define UBUS_VID_PID_METHOD "get_vidpid"
#define UBUS_GET_MODEMS_METHOD "get_modems"
#define UBUS_EXEC_FILE_METHOD "exec"
#define UBUS_REINIT_MODEMS_METHOD "reinit_modems"
#define MODEM_ID "id"
#define BUFFER_SIZE 128

// Supported modems.
typedef enum {
	HE910,
	LE910,
	LE910_V2,
	ME909U,
	EM820W,
	MC7354,
	EC20,
	EC25,
	UC20,
	MT421,
	ME909S,
	ME936,
	ME906S,
	MS2131,
	EG06,
	EG12,
	BG96,
	SLM750,
	EC200A,
	RG50X,
	RG52X,
	MODEM_DEV_UNSPECIFIED = -1 // add new devices above this line
} modem_dev;

typedef enum {
	PRIMARY,
	SECONDARY,
	INTERNAL,
	EXTERNAL,
	__MODEM_TYPE_CNT
} modem_type;

extern const char *modem_types[];

unsigned int get_modem(char *modem_id);
char *get_default_modem(void);
int ubus_get_modems(char **output);
void get_modems_callback(struct ubus_request *req, int type, struct blob_attr *msg);
int ubus_get_modem(char *modem_id, char *vid_pid);
void get_modem_callback(struct ubus_request *req, int type, struct blob_attr *msg);

void get_gpio_callback(struct ubus_request *req, int type, struct blob_attr *msg);
int gsmctl_modem_reboot(modem_dev device, char *output, char *modem_id,
                        int force_reinit);
int gsmctl_get_modem_type(char *modem_id, modem_type *type);
int modem_reinit(char *modem_id);

#ifdef __cplusplus
}
#endif
