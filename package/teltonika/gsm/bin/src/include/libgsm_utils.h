#pragma once

#include <stdbool.h>
#include <libtlt_uci.h>
#include <libgsm_modem.h>
#include <libsim.h>

#define MY_PIPE "/tmp/my_pipe"
#define NET_TIMEOUT 30

// Supported commands.
typedef enum {
	IP,
	CONNSTATE,
	PSSTATE,
	NETSTATE,
	IMEI,
	MODEL,
	MANUF,
	SERIALNUM,
	REVISION,
	MODEM_APPLICATION,
	IMSI,
	PINSTATE,
	SIMSTATE,
	SIGNAL,
	RSCP,
	ECIO,
	RSRP,
	SINR,
	RSRQ,
	CELLID,
	ICCID,
	OPERATOR,
	OPERNUM,
	CONNTYPE,
	BSENT,
	BRECV,
	ATCMD,
	TEMPERATURE,
	SMSREAD,
	SMSLIST,
	SMSMEM,
	SMSSEND,
	SMSSEND_PDU,
	SMSSEND_B64,
	SMSDEL,
	SHUTDOWN,
	PINCOUNT,
	NETWORK,
	SERVING,
	NEIGHBOUR,
	SIM_SWITCH,
	MODEM_REBOOT,
	USSD,
	MODEMTIME,
	SOFTWARE,
	BAND,
	VOLTE_STATUS,
	__REQUEST_BUFFER_INVALID
} request_buffer;

enum {
	RESTART_INIT,
	RESTART_INIT_BG,
	RELOAD_INIT,
	LUCI_RELOAD,
	REBOOT_FROM_SMS,
	RESET_TO_DEFAULT,
	UPGRADE,
	UPDATE_FOTA_FW,
	RESTART_SIM_CONF_SWITCH,
	RMS_RECONNECT,
	RESET_TO_USERDEFAULT,
	REBOOT_FROM_CALL
};

typedef enum {
	CONFIG,
	SECTION,
	OPTION,
	VALUE
} uci_action;


void get_interface_v6_callback(struct ubus_request *req, int type, struct blob_attr *msg);
void get_interface_v4_callback(struct ubus_request *req, int type, struct blob_attr *msg);

void check_alocate(void *name);

int system_function(char *service, int action);

char *get_time_stamp(void);
char *get_serial_number(void);
char *get_wired_wan_mac(void);
char *get_lan_mac(void);
char *get_current_fw_version(void);
char *get_wired_wan_ip_addr(void);
char *get_lan_ip_addr(void);
char *get_monitoring_status(void);
char *get_extended_monitoring_status(void);
char *get_available_fw_on_server(void);
char *get_mobile_ip_addresses(modem_dev device, char *info_modem_id);
char *get_info_from_gsm(modem_dev device, char *info_modem_id, request_buffer request);
char *get_sim_slot_in_use(char *info_modem_id);
char *get_new_line(void);
char *get_io_status(const char *param);
char *get_router_name(void);
char *prepare_databytes(long long int bytes);
lsim_t get_interface_sim(struct uci_context *uci, char *interface);
int get_mobile_interface_l3_device(char *interface, char **output);

//insert string required
void insert(char *str, size_t len, char c, size_t pos);

// Parse msg functions to get status.
char *read_msg(modem_dev device, char *curr_message, char *info_modem_id);
char *read_msg_event(modem_dev device, char *curr_message, char *info_modem_id, char *event_type,
		     char *event_text);
char *get_status_values(modem_dev device, char *param, char *info_modem_id);
char *get_status_values_event(modem_dev device, char *param, char *info_modem_id, char *event_type,
			      char *event_text);

int send_sms_multiple(modem_dev device, struct uci_context *uci, struct uci_section *section, const char *sender_nr, const char *text, const char *modem_id);

int exec_reboot(struct uci_context *uci, struct uci_section *section, char *number, int from_sms);
int exec_send_status(modem_dev device, char *sender_nr, struct uci_context *uci, struct uci_section *section);
int exec_send_monitoring_status(modem_dev device, char *senderNr, struct uci_context *uci, struct uci_section *section);
int exec_vpn_status(modem_dev device, char *senderNr, struct uci_context *uci, struct uci_section *section);

char *get_params(char *rule_path, int cut_new_line);
char *hex_to_str(char *str);
//void string_to_hex(char* str_pointer, int size);
int exec_send_iostatus(modem_dev device, char *senderNr, struct uci_context *uci, struct uci_section *section);
int exec_vpn_settings(modem_dev device, char *senderNr, struct uci_context *uci, struct uci_section *section, char *value, char *sms_text);
int exec_mobile_settings(struct uci_context *uci, struct uci_section *section, int i, char *paths[20]);
int exec_io_status(modem_dev device, char *sender_nr, struct uci_context *ctx, struct uci_section *sec);

int exec_rms_status_check(modem_dev device, char *sender_nr,
		struct uci_context *uci, struct uci_section *section);
int exec_rms_action(modem_dev device, char *sender_nr,
		    struct uci_context *uci, struct uci_section *section);

int exec_wifi_manager(struct uci_context *uci, struct uci_section *section);
int exec_mobile_manager(struct uci_context *uci, struct uci_section *section);

char *ucix_show_cfg(uci_action do_action, char *pac, char *sec, char *opt);
int exec_uci_show(char *text, char *sender_nr, modem_dev device, char *modem_id, struct uci_context *_uci, struct uci_section *_section);
int exec_uci_set(char *text, char *sender_nr, modem_dev device, char *modem_id, struct uci_context *_uci, struct uci_section *_section);
int exec_uci_get(char *command, char *sender_nr, modem_dev device, char *modem_id, struct uci_context *_uci, struct uci_section *_section);
int exec_uci_api(char *sms_text, char *sender_nr, modem_dev device, struct uci_context *uci, struct uci_section *section);
int exec_more(modem_dev device, char *senderNr, struct uci_context *uci, struct uci_section *section);

int exec_wol(char *senderNr, struct uci_context *uci, struct uci_section *section, modem_dev device);
int exec_send_profiles(modem_dev device, char *senderNr, struct uci_context *uci, struct uci_section *section);

int exec_gps_manager(struct uci_context *uci, struct uci_section *section);
int exec_gps_status(modem_dev device, struct uci_context *uci, struct uci_section *section, char *sender_nr);
int exec_change_profile(const char *profile);
int exec_data_limit(modem_dev device, char *sender_nr, struct uci_context *uci, struct uci_section *section);
int exec_data_usage_reset(modem_dev device, char *sender_nr, struct uci_context *uci, char *modem_id, char *interface);
int exec_ioset(modem_dev device, struct uci_context *uci,
	       struct uci_section *section);
int exec_config_update(modem_dev device, char *senderNr, struct uci_context *uci, struct uci_section *section);

int exec_ip_unblock(struct uci_context *uci, struct uci_section *section, char *sms_text, modem_dev device, char *sender_nr, char *modem_id);

int change_gpio_direction(int io, char dir);
int change_gpio_state(int io, int state);
int change_dwi_state(int io, char state);
int change_relay_state(int relay, int state);

int shell_escape(const char *input, char **output, bool enclose);
int realloc_char(char **orig, int size);
int curl_cmd(char *url, char *req_type, int conn_timeout, int verify_cert, char **data_name, char **data_value, int data_count);

int mdcollect_reset_usage(char *interface, lsim_t sim);
