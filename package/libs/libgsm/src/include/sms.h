#ifndef SMS_H
#define SMS_H

#include "modem.h"
#include "uci_function.h"

#include <assert.h>
#include <ctype.h>
#include <stdint.h>

#include <libgps/gps.h>
#include <libeventslog/libevents.h>
#include <libtlt_base/base.h>
#include <libtlt_socket_man/socket_man.h>

//reikalingi gsmctl_get_ip funkcijai
#include <netdb.h>
#include <ifaddrs.h>
#include "operator_list.h"

//reikia statuso gavimui
#include <time.h>
#include <json-c/json.h>
#include <libubus.h>
#include <libubox/blobmsg_json.h>
#include <regex.h>

#define VERSION "0.2b"

#define TIMEOUT_DEFAULT 6
#define TIMEOUT_SMSSEND 16
#define TIMEOUT_LONG 205

#define CME_10_ERR	10		// sim not inserted
#define CME_11_ERR	11		// sim pin required
#define CME_12_ERR	12		// sim puk required
#define CME_13_ERR	13		// sim failure
#define CME_14_ERR	14		// sim busy
#define CME_15_ERR	15		// sim wrong
#define CME_20_ERR	20		// memory full
#define CME_21_ERR	21		// invalid memory index
#define CME_22_ERR	22		// not found
#define CME_23_ERR	23		// memory failure
#define CME_30_ERR	30		// No network service failure
#define CME_100_ERR	100		// unknown error

#define CMS_300_ERR	300		// me failure
#define CMS_301_ERR	301		// sms service of me reserved
#define CMS_302_ERR	302		// operation not allowed
#define CMS_303_ERR	303		// operation not supported
#define CMS_304_ERR	304		// invalid pdu mode parameter
#define CMS_305_ERR	305		// invalid text mode parameter
#define CMS_310_ERR	310		// sim not inserted
#define CMS_311_ERR	311		// sim pin required
#define CMS_312_ERR	312		// ph-sim pin required
#define CMS_313_ERR	313		// sim failure
#define CMS_314_ERR	314		// sim busy
#define CMS_315_ERR	315		// sim wrong
#define CMS_316_ERR	316		// sim puk required
#define CMS_317_ERR	317		// sim pin2 required
#define CMS_318_ERR	318		// sim puk2 required
#define CMS_320_ERR	320		// memory failure
#define CMS_321_ERR	321		// invalid memory index
#define CMS_322_ERR	322		// memory full
#define CMS_500_ERR	500		// unknow error

#define BUFFER_SIZE 128
#define SMS_ARRAY_SIZE 10

#define UNSOLICITED_DATA "/tmp/unsolicited_data"

// Supported modes.
typedef enum {
	MODE_GSM,
	MODE_SMS,
	MODE_SHELL,
} APPMODE;

// SMS structure.
typedef struct {
	uint8_t SMSC_Length;
	uint8_t SMSC_Type;
	char SMSC_Number[32];
	int FO_SMSD;
	uint8_t AddrLength;
	uint8_t AddrType;
	char Sender[32];	// sms sender
	uint8_t TP_PID;
	uint8_t TP_DCS;
	char TimeStamp[24];	// sms time
	uint8_t DataLength;
	uint8_t DataheaderLength;
	char Data[0xff];		// sms text
} SMS_DATA;

// SMS structure full.
typedef struct {
	int Index;		// sms Index
	int Status;	// sms Status
	SMS_DATA stru_for_pdu;
} ONE_SMS;

// SMS structure with element number in it.
typedef struct {
	int num;
	int err;
	ONE_SMS  *list;
} SMS_LIST;

// Supported commands.
typedef enum {
	IP,
	CONNSTATE,
	NETSTATE,
	IMEI,
	MODEL,
	MANUF,
	SERIALNUM,
	REVISION,
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
	USSD
} request_buffer;

// GSM functions.
int gsmctl_send(int socket, char *command, char **output);
int gsmctl_get(int socket, char *command, char **output);
int gsmctl_get_ip(char *ifname, char **output);
int gsmctl_get_connstate(int socket, modem_dev device, char **output);
int gsmctl_get_netstate(int socket, modem_dev device, char **output);
int gsmctl_get_imsi(int socket, modem_dev device, char **output);
int gsmctl_get_iccid(int socket, modem_dev device, char **output);
int gsmctl_get_pinstate(int socket, modem_dev device, char **output);
int gsmctl_get_simstate(int socket, modem_dev device, char **output);
int gsmctl_get_signal_quality(int socket, modem_dev device, char **output);
int gsmctl_get_signal_rscp(int socket, modem_dev device, char **output); //-------
int gsmctl_get_signal_ecio(int socket, modem_dev device, char **output); //-------
int gsmctl_get_signal_rsrp(int socket, modem_dev device, char **output); //-------
int gsmctl_get_signal_sinr(int socket, modem_dev device, char **output); //-------
int gsmctl_get_signal_rsrq(int socket, modem_dev device, char **output); //-------
int gsmctl_get_signal_cell_id(int socket, modem_dev device, char **output); //-------
int gsmctl_get_operator(int socket, modem_dev device, char **output);
int gsmctl_get_opernum(int socket, modem_dev device, char **output);
int gsmctl_get_conntype(int socket, modem_dev device, char **output);
int gsmctl_get_iface_stats(char *ifname, const char *side, char **output);
int gsmctl_get_auxiliary(int socket, modem_dev device, char *cmd, char **output);
int gsmctl_get_imei(int socket, modem_dev device, char **output);
int gsmctl_get_model(int socket, modem_dev device, char **output);
int gsmctl_get_manuf(int socket, modem_dev device, char **output);
int gsmctl_get_revision(int socket, modem_dev device, char **output);
int gsmctl_get_serial(int socket, modem_dev device, char **output);
int gsmctl_get_mcc(int socket, modem_dev device, char **output);
int gsmctl_get_mnc(int socket, modem_dev device, char **output);
int gsmctl_get_lac(int socket, modem_dev device, char **output);
int gsmctl_get_temperature(int socket, modem_dev device, char **output);
int gsmctl_get_pincount(int socket, modem_dev device, char **output);
int gsmctl_get_network_info(int socket, modem_dev device, char **output);
int gsmctl_get_serving_cell(int socket, modem_dev device, char **output);
int gsmctl_get_neighbour_cells(int socket, modem_dev device, char **output);
void modem_answer_call(int socket_fd, modem_dev device);
void modem_reject_call(int socket, modem_dev device);
int gsmctl_set_ussd_command(int socket, modem_dev device, char *ussd_command, char **output);


// SMS functions.
int message_parser(char *pBuffer, SMS_DATA *pSMS);
int utf8_to_8859_converter(unsigned char *buffer, char *output, int sms_num, int sms_length);
unsigned char char_converter(unsigned int letter);
int create_pdu(char *number, char *text, size_t unicode, char *pdu, char nr_type[3], int all_sms, int sms_num, int random, int sms_length);
SMS_LIST gsmctl_get_sms_list_struct(int socket, modem_dev device, int index, char **output);
int gsmctl_get_sms(int socket, modem_dev device, int index, int total, char **output, SMS_DATA *sms_struct);
int gsmctl_get_sms_list(int socket, modem_dev device, char *list, char **output);
int gsmctl_get_sms_memory(int socket, modem_dev device, char **output);
int gsmctl_send_sms(int socket, modem_dev device, char *number, char *text, char **output);
int gsmctl_send_sms_pdu(int socket, modem_dev device, unsigned char *pdu, char **output);
int gsmctl_delete_sms(int socket, modem_dev device, int index, int total, char **output);
int gsmctl_scan_network(int socket, char **output);

// Based functions.
void check_error(int err_code, char **buffer);

//
void option_list_init(request_buffer *requests);

// Parse msg functions to get status.
char *read_msg(modem_dev device, char *text, char *event_type, char *event_text, char *section);
char *get_status_values(modem_dev device, char *param, char *event, char *event_txt, char *section);
char *custom_text(char *param, char *section);

//insert string required
void insert(char *str, size_t len, char c, size_t pos);

//ubus functions
int call_ubus_status(void);
void receive_call_result_data(struct ubus_request *req, int type, struct blob_attr *msg);
int get_regex_string(const char *command, char **resultString, const char *sourceString, int submatch);

// read connstate from unsolicited data file
int get_conn_state(void);

//reading gps coordinates
#define GPS_REQUEST "FIX_TIME LATITUDE LONGITUDE "
#define GPS_UNIX_SOCK_PATH "/tmp/gpsd.sock"
char *get_gps_coordinates_string(void);
int send_to_gpsd(char *);

#endif
