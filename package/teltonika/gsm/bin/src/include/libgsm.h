#ifdef __cplusplus
extern "C" {
#endif

#pragma once

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>

#include <liblog.h>
#include <libsms_limit.h>

//required for gsmctl_get_ip function
#include <netdb.h>
#include <ifaddrs.h>

#define __USE_XOPEN

//required for retrieving status
#include <time.h>
#include <regex.h>

#include <uci.h>
#include <libtlt_uci.h>

#include "libgsm_modem.h"
#include "libgsm_sms.h"
#include "libgsm_auth.h"
#include "libgsm_utils.h"
#include "libgsm_sim.h"
#include "libgsm_gps.h"
#include "libgsm_band.h"

#define VERSION "0.2b"

#define OK_ERR		 0
#define NO_ERR		-1
#define READ_ERR	-2
#define PACKET_ERR	-3
#define PARSE_ERR	-4
#define SYSCALL_ERR	-5
#define OPTION_ERR	-6
#define UCICALL_ERR	-7
#define IFADDRCALL_ERR	-8
#define MODEM_ERR	-9
#define SIM_ERR		-10
#define UNKNOWN_ERR	-11

#define BUFFER_SIZE 128
#define BUFFER_SIZE_2 256
#define BUFFER_SIZE_16 16
#define BUFFER_SIZE_64 64
#define BUFFER_SIZE_128 128
#define BUFFER_SIZE_256 256
#define BUFFER_SIZE_512 512

#define UNSOLICITED_DATA "/tmp/unsolicited_data"
#define NETWORK_UCI_PACKAGE "network"
#define CONFIG_PROTO "proto"
#define CONFIG_PDP "pdp"
#define UBUS_GSMD_AT_METHOD "at"

#define RUT200_NAME 	"RUT200"

enum {
	DO_NOTHING,
	CUT_NEW_LINE,
	REPLACE_WITH_SPACE
};


enum modem_states {
	MODEM_STATE_UNKNOWN,
	MODEM_STATE_READY,
	MODEM_STATE_SCANING,
	MODEM_STATE_SWITCHING,
	MODEM_STATE_CONNECTING,
	MODEM_STATE_SENDING_SMS,
};

// Supported modes.
typedef enum {
	MODE_GSM,
	MODE_SMS,
	MODE_SHELL,
} APPMODE;

// SMS Limit related stuff
enum sms_limit_id {
	SMS_LIMIT_DISABLED,
	SMS_LIMIT_MONTH,
	SMS_LIMIT_WEEK,
	SMS_LIMIT_DAY
};

struct sms_limit_info {
	enum sms_limit_id enabled;
	int limit_val;
};

static char *mcc_ban_list_rut200[] = {
	"276", "213", "505", "232", "206", "218", "284", "302", "219", "280", "230", "238", "248", "288",
	"244", "208", "547", "262", "266", "202", "216", "274", "272", "425", "222", "221", "247", "295",
	"246", "270", "294", "278", "340", "212", "297", "204", "362", "546", "530", "242", "260", "268",
	"226", "292", "220", "231", "293", "214", "240", "228", "234", "310", "312", "311", "316", "541",
	"250", "257",
};

static char *mcc_ban_list[] = {
	"250", "257",
};

//Debug functions
void set_debug_options(char dbg_lvl, char *app_name);

//UBUS functions
void get_output_callback(struct ubus_request *req, int type, struct blob_attr *msg);
void update_cgont_modems_status(char *modem_id, int state);
int ubus_call_network_action(char *ifname, char *action);
int find_and_down_interfaces(char *modem_id);

// GSM functions.
int gsmctl_send(char *command, char **output, char *modem_id);
int sms_send_to_gsmd(int pdu_size, char *pdu, char **output, char *modem_id);
int gsmctl_get(char *command, char *output, char *modem_id);
int gsmctl_get_ip(modem_dev device, char *ifname, char *output, char *modem_id);
int gsmctl_get_connstate(modem_dev device, char *output, char *modem_id);
int gsmctl_get_netstate(modem_dev device, char *output, char *modem_id);
int gsmctl_get_imsi(modem_dev device, char *output, char *modem_id);
int gsmctl_get_iccid(modem_dev device, char *output, char *modem_id);
int gsmctl_get_pinstate(modem_dev device, char *output, char *modem_id);
int gsmctl_get_simstate(modem_dev device, char *output, char *modem_id);
int gsmctl_get_signal_quality(modem_dev device, char *output, char *modem_id);
int gsmctl_get_signal_rscp(modem_dev device, char *output, char *modem_id);
int gsmctl_get_signal_ecio(modem_dev device, char *output, char *modem_id);
int gsmctl_get_signal_rsrp(modem_dev device, char *output, char *modem_id);
int gsmctl_get_signal_sinr(modem_dev device, char *output, char *modem_id);
int gsmctl_get_signal_rsrq(modem_dev device, char *output, char *modem_id);
int gsmctl_get_signal_cell_id(modem_dev device, char *output, char *modem_id);
int gsmctl_get_operator(modem_dev device, char *output, char *modem_id);
int gsmctl_get_opernum(modem_dev device, char *output, char *modem_id);
int gsmctl_get_conntype(modem_dev device, char *output, char *modem_id);
int gsmctl_get_modem_time(modem_dev device, int timezone_type, char *output, char *modem_id);
int gsmctl_get_iface_stats(char *ifname, const char *side, char *output);
int gsmctl_get_auxiliary(char *cmd, char **output, char *modem_id);
int gsmctl_get_imei(modem_dev device, char *output, char *modem_id);
int gsmctl_get_model(modem_dev device, char *output, char *modem_id);
int gsmctl_get_manuf(modem_dev device, char *output, char *modem_id);
int gsmctl_get_revision(modem_dev device, char *output, char *modem_id);
int gsmctl_get_modem_application_firmware(modem_dev device, char *output, char *modem_id);
int gsmctl_get_software_version(modem_dev device, char *output, char *modem_id);
int gsmctl_get_serial(modem_dev device, char *output, char *modem_id);
int gsmctl_get_mcc(modem_dev device, char *output, char *modem_id);
int gsmctl_get_mnc(modem_dev device, char *output, char *modem_id);
int gsmctl_get_lac(modem_dev device, char *output, char *modem_id);
int gsmctl_get_temperature(modem_dev device, char *output, char *modem_id);
int gsmctl_get_pincount(modem_dev device, char *output, char *modem_id);
int gsmctl_get_network_info(modem_dev device, char **output, char *modem_id);
int gsmctl_get_serving_cell(modem_dev device, char *output, char *modem_id);
int gsmctl_get_neighbour_cells(modem_dev device, char **output, char *modem_id);
int gsmctl_shutdown(modem_dev device, char *output, char *modem_id);
int gsmctl_get_actstate(modem_dev device, char *output, char *modem_id);
int gsmctl_scan_network(char **output, char *modem_id);
int gsmctl_get_data_counter(modem_dev device, char *output, char *modem_id, uint64_t *tx, uint64_t *rx);
int gsmctl_get_ps_status(char *output, char *modem_id);
int modem_answer_call(char *modem_id);
int modem_reject_call(char *modem_id);

char *strdup_nth_element(const char *string, const char *delimiter,
			 const int n);

int gsmctl_force_connect_to_operator(char *operator_number, char *output, char *modem_id);
int gsmctl_manual_auto_connect_to_operator(char *operator_number, char *output, char *modem_id);
int gsmctl_set_cfun(int fun, char *output, char *modem_id);

// GSM parse functions
int gsmctl_get_netstate_parser_ex(modem_dev device, char **output);
int better_signal_quality_parser(modem_dev device, char *msg, int *signal);
int gsmctl_get_signal_quality_parser(modem_dev device, char **output);
int gsmctl_get_signal_quality_parser_ex(modem_dev device, char **output);
int gsmctl_get_conntype_parser(modem_dev device, char **output);

int set_sim_pin(modem_dev device, char *output, char *modem_id, char *simpin);

// internal gsm functions
int gsmctl_sms_limit(struct ubus_context *ctx, int sim, char *modem_id);
void libgsm_log_event(log_table table, event_priority priority, char *text, char *sender);

// Based functions.
int check_iface_proto(char *iface);
int check_iface_context_id(char *iface, char *modem_id);

void check_error(int err_code, char *buffer);
int check_output_errors(char *output);

void option_list_init(request_buffer *requests);

// Helper functions
int prevent_from_modem_restart_loop(char *flag_name, char *modem_id);
int modem_gpio_reset(char *modem_id);
void modem_reboot_via_at(char *modem_id);

#ifdef __cplusplus
}
#endif
