// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// This sample code works with the Remote Monitoring solution accelerator and shows how to:
// - Report device capabilities.
// - Send telemetry.
// - Respond to methods, including a long-running firmware update method.
// The code simulates a simple Chiller device.

// CAVEAT: This sample is to demonstrate azure IoT client concepts only and is not a guide design principles or style
// Checking of return codes and error values are omitted for brevity.  Please practice sound engineering practices 
// when writing production code.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include "cJSON.h"
#include "iothub.h"
#include "iothub_device_client.h"
#include "iothub_client_options.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "iothubtransportmqtt.h"
#include "parson.h"
#include "certs.c"
#include <libgsm/sms.h>
#include <libgsm/modem.h>
#include <sys/socket.h>
#define UNIX_SOCK_PATH "/tmp/gsmd.sock"
#define UNIX_SOCK_PATH2 "/tmp/azure.sock"
#include "azure_iothub.h"
#include <mosquitto.h>
#define BUF_SIZE 64

char *appname = "rms_json";
unsigned char nolog = 0;

#define MESSAGERESPONSE(code, message) const char deviceMethodResponse[] = message; \
	*response_size = sizeof(deviceMethodResponse) - 1;                              \
	*response = malloc(*response_size);                                             \
	(void)memcpy(*response, deviceMethodResponse, *response_size);                  \
	result = code;                                                                  \

#define FIRMWARE_UPDATE_STATUS_VALUES \
    DOWNLOADING,                      \
    APPLYING,                         \
    REBOOTING,                        \
    IDLE                              \

/*Enumeration specifying firmware update status */
DEFINE_ENUM(FIRMWARE_UPDATE_STATUS, FIRMWARE_UPDATE_STATUS_VALUES);
DEFINE_ENUM_STRINGS(FIRMWARE_UPDATE_STATUS, FIRMWARE_UPDATE_STATUS_VALUES);

/* Paste in your device connection string  */
static char msgText[1024];
static size_t g_message_count_send_confirmations = 0;
static const char* initialFirmwareVersion = "1.0.0";

IOTHUB_DEVICE_CLIENT_HANDLE device_handle;

// <datadefinition>
typedef struct MESSAGESCHEMA_TAG
{
	char* name;
	char* format;
	char* fields;
} MessageSchema;

struct dynamic_parameters
{
	char *ipaddr;
	char *bsent;
	char *brecv;
	char *connstate;
	char *netstate;
	char *imei;
	char *iccid;
	char *model;
	char *manuf;
	char *serial;
	char *revision;
	char *imsi;
	char *simstate;
	char *pinstate;
	char *gsm_signal;
	char *rscp;
	char *ecio;
	char *rsrp;
	char *sinr;
	char *rsrq;
	char *cellid;
	char *operator;
    char *opernum;
	char *conntype;
	char *temp;
	char *pincount;
};

typedef struct TELEMETRYSCHEMA_TAG
{
	char* interval;
	char* messageTemplate;
	MessageSchema messageSchema;
} TelemetrySchema;

typedef struct TELEMETRYPROPERTIES_TAG
{
	TelemetrySchema dynamicSchema;
} TelemetryProperties;

typedef struct CHILLER_TAG
{
	// Reported properties
	char* protocol;
	char* supportedMethods;
	char* type;
	char* firmware;
	FIRMWARE_UPDATE_STATUS firmwareUpdateStatus;
	char* location;
	double latitude;
	double longitude;
	TelemetryProperties telemetry;

	// Manage firmware update process
	char* new_firmware_version;
	char* new_firmware_URI;
} Chiller;
// </datadefinition>

/*  Converts the Chiller object into a JSON blob with reported properties ready to be sent across the wire as a twin. */
static char* serializeToJson(Chiller* chiller)
{
	char* result;

	JSON_Value* root_value = az_json_value_init_object();
	JSON_Object* root_object = az_json_value_get_object(root_value);

	// Only reported properties:
	(void)az_json_object_set_string(root_object, "Protocol", chiller->protocol);
	(void)az_json_object_set_string(root_object, "SupportedMethods", chiller->supportedMethods);
	(void)az_json_object_set_string(root_object, "Type", chiller->type);
	(void)az_json_object_dotset_string(root_object, "Telemetry.dynamicSchema.Interval", chiller->telemetry.dynamicSchema.interval);
	(void)az_json_object_dotset_string(root_object, "Telemetry.dynamicSchema.MessageTemplate", chiller->telemetry.dynamicSchema.messageTemplate);
	(void)az_json_object_dotset_string(root_object, "Telemetry.dynamicSchema.MessageSchema.Name", chiller->telemetry.dynamicSchema.messageSchema.name);
	(void)az_json_object_dotset_string(root_object, "Telemetry.dynamicSchema.MessageSchema.Format", chiller->telemetry.dynamicSchema.messageSchema.format);
	(void)az_json_object_dotset_string(root_object, "Telemetry.dynamicSchema.MessageSchema.Fields", chiller->telemetry.dynamicSchema.messageSchema.fields);
	result = az_json_serialize_to_string(root_value);

	az_json_value_free(root_value);

	return result;
}

static void connection_status_callback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* user_context)
{
	(void)reason;
	(void)user_context;
	// This sample DOES NOT take into consideration network outages.
	if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
	{
		syslog(LOG_INFO, "The device client is connected to iothub");
	}
	else
	{
		syslog(LOG_INFO, "The device client has been disconnected");
	}
}

static void send_confirm_callback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
	(void)userContextCallback;
	g_message_count_send_confirmations++;
	(void)printf("Confirmation callback received for message %zu with result %s\r\n", g_message_count_send_confirmations, ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
	syslog(LOG_INFO, "Confirmation callback received for message %zu with result %s\r\n", g_message_count_send_confirmations, ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
}

static void reported_state_callback(int status_code, void* userContextCallback)
{
	(void)userContextCallback;
	(void)printf("Device Twin reported properties update completed with result: %d\r\n", status_code);
}

static void sendChillerReportedProperties(Chiller* chiller)
{
	if (device_handle != NULL)
	{
		char* reportedProperties = serializeToJson(chiller);
		(void)IoTHubDeviceClient_SendReportedState(device_handle, (const unsigned char*)reportedProperties, strlen(reportedProperties), reported_state_callback, NULL);
		free(reportedProperties);
	}
}

// <devicemethodcallback>
static int device_method_callback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* response_size, void* userContextCallback)
{
	Chiller *chiller = (Chiller *)userContextCallback;

	int result;

	(void)printf("Direct method name:    %s\r\n", method_name);

	(void)printf("Direct method payload: %.*s\r\n", (int)size, (const char*)payload);

	if (strcmp("Reboot", method_name) == 0)
	{
		MESSAGERESPONSE(201, "{ \"Response\": \"Rebooting\" }")
	}
	else if (strcmp("EmergencyValveRelease", method_name) == 0)
	{
		MESSAGERESPONSE(201, "{ \"Response\": \"Releasing emergency valve\" }")
	}
	else if (strcmp("IncreasePressure", method_name) == 0)
	{
		MESSAGERESPONSE(201, "{ \"Response\": \"Increasing pressure\" }")
	}
	else
	{
		// All other entries are ignored.
		(void)printf("Method not recognized\r\n");
		MESSAGERESPONSE(400, "{ \"Response\": \"Method not recognized\" }")
	}

	return result;
}
// </devicemethodcallback>

// <sendmessage>
static void send_message(IOTHUB_DEVICE_CLIENT_HANDLE handle, char* message, char* schema)
{
	IOTHUB_MESSAGE_HANDLE message_handle = IoTHubMessage_CreateFromString(message);

	// Set system properties
	(void)IoTHubMessage_SetMessageId(message_handle, "MSG_ID");
	(void)IoTHubMessage_SetCorrelationId(message_handle, "CORE_ID");
	(void)IoTHubMessage_SetContentTypeSystemProperty(message_handle, "application%2fjson");
	(void)IoTHubMessage_SetContentEncodingSystemProperty(message_handle, "utf-8");

	// Set application properties
	MAP_HANDLE propMap = IoTHubMessage_Properties(message_handle);
	(void)Map_AddOrUpdate(propMap, "$$MessageSchema", schema);
	(void)Map_AddOrUpdate(propMap, "$$ContentType", "JSON");

	time_t now = time(0);
	struct tm* timeinfo;
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996) /* Suppress warning about possible unsafe function in Visual Studio */
#endif
	timeinfo = gmtime(&now);
#ifdef _MSC_VER
#pragma warning(pop)
#endif
	char timebuff[50];
	strftime(timebuff, 50, "%Y-%m-%dT%H:%M:%SZ", timeinfo);
	(void)Map_AddOrUpdate(propMap, "$$CreationTimeUtc", timebuff);

	IoTHubDeviceClient_SendEventAsync(handle, message_handle, send_confirm_callback, NULL);

	IoTHubMessage_Destroy(message_handle);
}
// </sendmessage>



cJSON *getData(modem_dev device, int socket_fd, cJSON *root, struct dynamic_parameters dyn_data)
{
	if (strcmp(ipaddr, "1") == 0){
		dyn_data.ipaddr = (char *) malloc(100 * sizeof(char));
		dyn_data.ipaddr[0] = 0;
		gsmctl_get_ip("wwan0", &dyn_data.ipaddr);
		dyn_data.ipaddr[strlen(dyn_data.ipaddr)] = 0;
		cJSON_AddStringToObject(root, "ip", dyn_data.ipaddr);
		free(dyn_data.ipaddr);
		dyn_data.ipaddr = NULL;
	}

	if (strcmp(bsent, "1") == 0){
		dyn_data.bsent = (char *) malloc(100 * sizeof(char));
		dyn_data.bsent[0] = 0;
		gsmctl_get_iface_stats("wwan0", "tx", &dyn_data.bsent);
		dyn_data.bsent[strlen(dyn_data.bsent)-1] = 0;
		cJSON_AddStringToObject(root, "bytes_sent", dyn_data.bsent);
		free(dyn_data.bsent);
		dyn_data.bsent = NULL;
	}

	if (strcmp(brecv, "1") == 0){
		dyn_data.brecv = (char *) malloc(100 * sizeof(char));
		dyn_data.brecv[0] = 0;
		gsmctl_get_iface_stats("wwan0", "rx", &dyn_data.brecv);
		dyn_data.brecv[strlen(dyn_data.brecv)-1] = 0;
		cJSON_AddStringToObject(root, "bytes_received", dyn_data.brecv);
		free(dyn_data.brecv);
		dyn_data.brecv = NULL;
	}

	if (strcmp(connstate, "1") == 0){
		dyn_data.connstate = (char *) malloc(100 * sizeof(char));
		dyn_data.connstate[0] = 0;
		gsmctl_get_connstate(socket_fd, device, &dyn_data.connstate);
		dyn_data.connstate[strlen(dyn_data.connstate)-1] = 0;
		cJSON_AddStringToObject(root, "connection_state", dyn_data.connstate);
		free(dyn_data.connstate);
		dyn_data.connstate = NULL;
	}

	if (strcmp(netstate, "1") == 0){
		dyn_data.netstate = (char *) malloc(100 * sizeof(char));
		dyn_data.netstate[0] = 0;
		gsmctl_get_netstate(socket_fd, device, &dyn_data.netstate);
		dyn_data.netstate[strlen(dyn_data.netstate)-1] = 0;
		cJSON_AddStringToObject(root, "network_state", dyn_data.netstate);
		free(dyn_data.netstate);
		dyn_data.netstate = NULL;
	}

	if (strcmp(imei, "1") == 0){
		dyn_data.imei = (char *) malloc(100 * sizeof(char));
		dyn_data.imei[0] = 0;
		gsmctl_get_imei(socket_fd, device, &dyn_data.imei);
		dyn_data.imei[strlen(dyn_data.imei)-1] = 0;
		cJSON_AddStringToObject(root, "imei", dyn_data.imei);
		free(dyn_data.imei);
		dyn_data.imei = NULL;
	}

	if (strcmp(iccid, "1") == 0){
		dyn_data.iccid = (char *) malloc(100 * sizeof(char));
		dyn_data.iccid[0] = 0;
		gsmctl_get_iccid(socket_fd, device, &dyn_data.iccid);
		dyn_data.iccid[strlen(dyn_data.iccid)-1] = 0;
		cJSON_AddStringToObject(root, "iccid", dyn_data.iccid);
		free(dyn_data.iccid);
		dyn_data.iccid = NULL;
	}

	if (strcmp(model, "1") == 0){
		dyn_data.model = (char *) malloc(100 * sizeof(char));
		dyn_data.model[0] = 0;
		gsmctl_get_model(socket_fd, device, &dyn_data.model);
		dyn_data.model[strlen(dyn_data.model)-1] = 0;
		cJSON_AddStringToObject(root, "model", dyn_data.model);
		free(dyn_data.model);
		dyn_data.model = NULL;
	}

	if (strcmp(manuf, "1") == 0){
		dyn_data.manuf = (char *) malloc(100 * sizeof(char));
		dyn_data.manuf[0] = 0;
		gsmctl_get_manuf(socket_fd, device, &dyn_data.manuf);
		dyn_data.manuf[strlen(dyn_data.manuf)-1] = 0;
		cJSON_AddStringToObject(root, "manufactorer", dyn_data.manuf);
		free(dyn_data.manuf);
		dyn_data.manuf = NULL;
	}

	if (strcmp(serial, "1") == 0){
		dyn_data.serial = (char *) malloc(100 * sizeof(char));
		dyn_data.serial[0] = 0;
		gsmctl_get_serial(socket_fd, device, &dyn_data.serial);
		dyn_data.serial[strlen(dyn_data.serial)-1] = 0;
		cJSON_AddStringToObject(root, "serial", dyn_data.serial);
		free(dyn_data.serial);
		dyn_data.serial = NULL;
	}

	if (strcmp(revision, "1") == 0){
		dyn_data.revision = (char *) malloc(100 * sizeof(char));
		dyn_data.revision[0] = 0;
		gsmctl_get_revision(socket_fd, device, &dyn_data.revision);
		dyn_data.revision[strlen(dyn_data.revision)-1] = 0;
		cJSON_AddStringToObject(root, "revision", dyn_data.revision);
		free(dyn_data.revision);
		dyn_data.revision = NULL;
	}

	if (strcmp(imsi, "1") == 0){
		dyn_data.imsi = (char *) malloc(100 * sizeof(char));
		dyn_data.imsi[0] = 0;
		gsmctl_get_imsi(socket_fd, device, &dyn_data.imsi);
		dyn_data.imsi[strlen(dyn_data.imsi)-1] = 0;
		cJSON_AddStringToObject(root, "imsi", dyn_data.imsi);
		free(dyn_data.imsi);
		dyn_data.imsi = NULL;
	}

	if (strcmp(simstate, "1") == 0){
		dyn_data.simstate = (char *) malloc(100 * sizeof(char));
		dyn_data.simstate[0] = 0;
		gsmctl_get_simstate(socket_fd, device, &dyn_data.simstate);
		dyn_data.simstate[strlen(dyn_data.simstate)-1] = 0;
		cJSON_AddStringToObject(root, "sim_state", dyn_data.simstate);
		free(dyn_data.simstate);
		dyn_data.simstate = NULL;
	}

	if (strcmp(pinstate, "1") == 0){
		dyn_data.pinstate = (char *) malloc(100 * sizeof(char));
		dyn_data.pinstate[0] = 0;
		gsmctl_get_pinstate(socket_fd, device, &dyn_data.pinstate);
		dyn_data.pinstate[strlen(dyn_data.pinstate)-1] = 0;
		cJSON_AddStringToObject(root, "pin_state", dyn_data.pinstate);
		free(dyn_data.pinstate);
		dyn_data.pinstate = NULL;
	}

	if (strcmp(gsm_signal, "1") == 0){
		dyn_data.gsm_signal = (char *) malloc(100 * sizeof(char));
		dyn_data.gsm_signal[0] = 0;
		gsmctl_get_signal_quality(socket_fd, device, &dyn_data.gsm_signal);
		dyn_data.gsm_signal[strlen(dyn_data.gsm_signal)-1] = 0;
		cJSON_AddStringToObject(root, "signal", dyn_data.gsm_signal);
		free(dyn_data.gsm_signal);
		dyn_data.gsm_signal = NULL;
	}

	if (strcmp(rscp, "1") == 0){
		dyn_data.rscp = (char *) malloc(100 * sizeof(char));
		dyn_data.rscp[0] = 0;
		gsmctl_get_signal_rscp(socket_fd, device, &dyn_data.rscp);
		dyn_data.rscp[strlen(dyn_data.rscp)-1] = 0;
		cJSON_AddStringToObject(root, "rscp", dyn_data.rscp);
		free(dyn_data.rscp);
		dyn_data.rscp = NULL;
	}

	if (strcmp(ecio, "1") == 0){
		dyn_data.ecio = (char *) malloc(100 * sizeof(char));
		dyn_data.ecio[0] = 0;
		gsmctl_get_signal_ecio(socket_fd, device, &dyn_data.ecio);
		dyn_data.ecio[strlen(dyn_data.ecio)-1] = 0;
		cJSON_AddStringToObject(root, "ecio", dyn_data.ecio);
		free(dyn_data.ecio);
		dyn_data.ecio = NULL;
	}

	if (strcmp(rsrp, "1") == 0){
		dyn_data.rsrp = (char *) malloc(100 * sizeof(char));
		dyn_data.rsrp[0] = 0;
		gsmctl_get_signal_rsrp(socket_fd, device, &dyn_data.rsrp);
		dyn_data.rsrp[strlen(dyn_data.rsrp)-1] = 0;
		cJSON_AddStringToObject(root, "rsrp", dyn_data.rsrp);
		free(dyn_data.rsrp);
		dyn_data.rsrp = NULL;
	}

	if (strcmp(sinr, "1") == 0){
		dyn_data.sinr = (char *) malloc(100 * sizeof(char));
		dyn_data.sinr[0] = 0;
		gsmctl_get_signal_sinr(socket_fd, device, &dyn_data.sinr);
		dyn_data.sinr[strlen(dyn_data.sinr)-1] = 0;
		cJSON_AddStringToObject(root, "sinr", dyn_data.sinr);
		free(dyn_data.sinr);
		dyn_data.sinr = NULL;
	}

	if (strcmp(rsrq, "1") == 0){
		dyn_data.rsrq = (char *) malloc(100 * sizeof(char));
		dyn_data.rsrq[0] = 0;
		gsmctl_get_signal_rsrq(socket_fd, device, &dyn_data.rsrq);
		dyn_data.rsrq[strlen(dyn_data.rsrq)-1] = 0;
		cJSON_AddStringToObject(root, "rsrq", dyn_data.rsrq);
		free(dyn_data.rsrq);
		dyn_data.rsrq = NULL;
	}

	if (strcmp(cellid, "1") == 0){
		dyn_data.cellid = (char *) malloc(100 * sizeof(char));
		dyn_data.cellid[0] = 0;
		gsmctl_get_signal_cell_id(socket_fd, device, &dyn_data.cellid);
		dyn_data.cellid[strlen(dyn_data.cellid)-1] = 0;
		cJSON_AddStringToObject(root, "cell_id", dyn_data.cellid);
		free(dyn_data.cellid);
		dyn_data.cellid = NULL;
	}

	if (strcmp(operator, "1") == 0){
		dyn_data.operator = (char *) malloc(100 * sizeof(char));
		dyn_data.operator[0] = 0;
		gsmctl_get_operator(socket_fd, device, &dyn_data.operator);
		dyn_data.operator[strlen(dyn_data.operator)-1] = 0;
		cJSON_AddStringToObject(root, "operator", dyn_data.operator);
		free(dyn_data.operator);
		dyn_data.operator = NULL;
	}

	if (strcmp(opernum, "1") == 0){
		dyn_data.opernum = (char *) malloc(100 * sizeof(char));
		dyn_data.opernum[0] = 0;
		gsmctl_get_opernum(socket_fd, device, &dyn_data.opernum);
		dyn_data.opernum[strlen(dyn_data.opernum)-1] = 0;
		cJSON_AddStringToObject(root, "operator_number", dyn_data.opernum);
		free(dyn_data.opernum);
		dyn_data.opernum = NULL;
	}

	if (strcmp(conntype, "1") == 0){
		dyn_data.conntype = (char *) malloc(100 * sizeof(char));
		dyn_data.conntype[0] = 0;
		gsmctl_get_conntype(socket_fd, device, &dyn_data.conntype);
		dyn_data.conntype[strlen(dyn_data.conntype)-1] = 0;
		cJSON_AddStringToObject(root, "connection_type", dyn_data.conntype);
		free(dyn_data.conntype);
		dyn_data.conntype = NULL;
	}

	if (strcmp(temp, "1") == 0){
		dyn_data.temp = (char *) malloc(100 * sizeof(char));
		dyn_data.temp[0] = 0;
		gsmctl_get_temperature(socket_fd, device, &dyn_data.temp);
		dyn_data.temp[strlen(dyn_data.temp)-1] = 0;
		cJSON_AddStringToObject(root, "temperature", dyn_data.temp);
		free(dyn_data.temp);
		dyn_data.temp = NULL;
	}

	if (strcmp(pincount, "1") == 0){
		dyn_data.pincount = (char *) malloc(100 * sizeof(char));
		dyn_data.pincount[0] = 0;
		gsmctl_get_pincount(socket_fd, device, &dyn_data.pincount);
		dyn_data.pincount[strlen(dyn_data.pincount)-1] = 0;
		cJSON_AddStringToObject(root, "pin_count", dyn_data.pincount);
		free(dyn_data.pincount);
		dyn_data.pincount = NULL;
	}

	return root;
}

void readUciConfigs()
{
	//uci = uci_init();
	char *data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "ipaddr");
	memset(ipaddr, '\0', sizeof(ipaddr));
	strncpy(ipaddr, data, sizeof(ipaddr));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "bsent");
	memset(bsent, '\0', sizeof(bsent));
	strncpy(bsent, data, sizeof(bsent));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "brecv");
	memset(brecv, '\0', sizeof(brecv));
	strncpy(brecv, data, sizeof(brecv));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "connstate");
	memset(connstate, '\0', sizeof(connstate));
	strncpy(connstate, data, sizeof(connstate));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "netstate");
	memset(netstate, '\0', sizeof(netstate));
	strncpy(netstate, data, sizeof(netstate));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "imei");
	memset(imei, '\0', sizeof(imei));
	strncpy(imei, data, sizeof(imei));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "iccid");
	memset(iccid, '\0', sizeof(iccid));
	strncpy(iccid, data, sizeof(iccid));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "model");
	memset(model, '\0', sizeof(model));
	strncpy(model, data, sizeof(model));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "manuf");
	memset(manuf, '\0', sizeof(manuf));
	strncpy(manuf, data, sizeof(manuf));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "serial");
	memset(serial, '\0', sizeof(serial));
	strncpy(serial, data, sizeof(serial));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "revision");
	memset(revision, '\0', sizeof(revision));
	strncpy(revision, data, sizeof(revision));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "imsi");
	memset(imsi, '\0', sizeof(imsi));
	strncpy(imsi, data, sizeof(imsi));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "simstate");
	memset(simstate, '\0', sizeof(simstate));
	strncpy(simstate, data, sizeof(simstate));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "pinstate");
	memset(pinstate, '\0', sizeof(pinstate));
	strncpy(pinstate, data, sizeof(pinstate));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "signal");
	memset(gsm_signal, '\0', sizeof(gsm_signal));
	strncpy(gsm_signal, data, sizeof(gsm_signal));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "rscp");
	memset(rscp, '\0', sizeof(rscp));
	strncpy(rscp, data, sizeof(rscp));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "ecio");
	memset(ecio, '\0', sizeof(ecio));
	strncpy(ecio, data, sizeof(ecio));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "rsrp");
	memset(rsrp, '\0', sizeof(rsrp));
	strncpy(rsrp, data, sizeof(rsrp));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "sinr");
	memset(sinr, '\0', sizeof(sinr));
	strncpy(sinr, data, sizeof(sinr));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "rsrq");
	memset(rsrq, '\0', sizeof(rsrq));
	strncpy(rsrq, data, sizeof(rsrq));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "cellid");
	memset(cellid, '\0', sizeof(cellid));
	strncpy(cellid, data, sizeof(cellid));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "operator");
	memset(operator, '\0', sizeof(operator));
	strncpy(operator, data, sizeof(operator));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "opernum");
	memset(opernum, '\0', sizeof(opernum));
	strncpy(opernum, data, sizeof(opernum));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "conntype");
	memset(conntype, '\0', sizeof(conntype));
	strncpy(conntype, data, sizeof(conntype));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "temp");
	memset(temp, '\0', sizeof(temp));
	strncpy(temp, data, sizeof(temp));

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "pincount");
	memset(pincount, '\0', sizeof(pincount));
	strncpy(pincount, data, sizeof(pincount));

	if (data)
		free(data);
	data = NULL;
}

void send_data(int timeout, char* schema)
{
    cJSON *root;
    struct sockaddr_un remote;
    int socket_fd = -1, len;
    struct dynamic_parameters dyn_data;
    char *out;
    modem_dev device;
    device = get_modem();

    while (1) {
        root = cJSON_CreateObject();
        if ((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(1);
        }

        remote.sun_family = AF_UNIX;
        strcpy(remote.sun_path, UNIX_SOCK_PATH);
        len = strlen(remote.sun_path) + sizeof(remote.sun_family);
        if (connect(socket_fd, (struct sockaddr *) &remote, len) == -1) {
            perror("socket connection error");
            exit(1);
        }
        root = getData(device, socket_fd, root, dyn_data);

        out = cJSON_Print(root);
        cJSON_Delete(root);

        send_message(device_handle, out, schema);
        printf(out);
        close(socket_fd);

        ThreadAPI_Sleep(timeout * 1000);
        free(out);
    }
}

void open_server_socket(char* schema, char* custom_socket_path)
{
    //cJSON *root;
    //char *out;
    char buffer[1024] = {0};
    struct sockaddr_un remote;
    int socket_fd = -1, new_socket = -1;
    int addrlen = sizeof(remote);

	unlink(custom_socket_path);

    memset(&remote, 0, sizeof(struct sockaddr_un));  /* Clear address structure */
    remote.sun_family = AF_UNIX;

    strncpy(&remote.sun_path[0], custom_socket_path, 128);

    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd == -1)
        exit(1);

    if (bind(socket_fd, (struct sockaddr *) &remote,
             sizeof(sa_family_t) + strlen(custom_socket_path) + 1) == -1)
        exit(1);

    while(1) {
		if (listen(socket_fd, 3) < 0) {
			perror("listen");
			exit(1);
		}

		if ((new_socket = accept(socket_fd, (struct sockaddr *)&remote,
								 (socklen_t*)&addrlen))<0)
		{
			perror("accept");
			exit(1);
		}
        //root = cJSON_CreateObject();
        read(new_socket, buffer, 1024);
        //cJSON_AddStringToObject(root, "message", buffer);
        //out = cJSON_Print(root);
        //printf(out);
        printf(buffer);
        send_message(device_handle, buffer, schema);
        //cJSON_Delete(root);
        //free(out);
    }
    close(socket_fd);
    close(new_socket);

}

void connect_callback(struct mosquitto *mosq, void *userdata,
					  int result) {

	if (!result) {
		mosquitto_subscribe(mosq, NULL, topic, 2);
		syslog(LOG_INFO, "Subscribing to: %s", topic);
	}
	else
		fprintf(stderr, "Connect failed\n");
}

void message_callback(struct mosquitto *mosq, void *userdata,
					  const struct mosquitto_message *message) {
	cJSON *root;
	char *out;

	if (message->payloadlen) {
		char value[BUF_SIZE];

		printf("Got request: %s %s\n", message->topic,
			   message->payload);

		if (!strcmp(message->topic, topic)) {
			printf("Send response: %s %s \n", topic, message->payload);
			root = cJSON_CreateObject();
			cJSON_AddStringToObject(root, "value", message->payload);
			out = cJSON_Print(root);
			cJSON_Delete(root);

			send_message(device_handle, out, "dynamic-information;v1");
			printf(out);
			free(out);
		}
		else
			printf("Command not found\n");
	}
}
void init_mqtt_config() {
	char *data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "mqtt_ip");
	if (data) {
		memset(mqtt_host, '\0', sizeof(mqtt_host));
		strncpy(mqtt_host, data, sizeof(mqtt_host));
		free(data);
	} else {
		syslog(LOG_INFO, "Missing MQTT host");
		exit(1);
	}
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "mqtt_port");
	if (data) {
		mqtt_port = atoi(data);
		free(data);
	} else {
		syslog(LOG_INFO, "Missing MQTT port");
		exit(1);
	}
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "mqtt_username");
	if (data) {
		memset(mqtt_username, '\0', sizeof(mqtt_username));
		strncpy(mqtt_username, data, sizeof(mqtt_username));
		free(data);
	}
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "mqtt_password");
	if (data) {
		memset(mqtt_password, '\0', sizeof(mqtt_password));
		strncpy(mqtt_password, data, sizeof(mqtt_password));
		free(data);
	}
	data = NULL;

	data = ucix_get_option(uci, "azure_iothub", "azure_iothub", "mqtt_topic");
	if (data) {
		memset(topic, '\0', sizeof(topic));
		strncpy(topic, data, sizeof(topic));
		free(data);
	} else {
		syslog(LOG_INFO, "Missing MQTT topic");
		exit(1);
	}
	data = NULL;
}

void subscriber_main()
{
	struct mosquitto *mosq = NULL;
	int c;

	init_mqtt_config();

	if ((mqtt_username && !mqtt_password) ||
		(mqtt_password && !mqtt_username)) {
		syslog(LOG_INFO, "Missing MQTT username or password");
		exit(EXIT_FAILURE);
	}

	mosquitto_lib_init();
	mosq = mosquitto_new(NULL, true, NULL);

	if (!mosq) {
		fprintf(stderr, "%s: out of memory.\n", __FUNCTION__);
		exit(EXIT_FAILURE);
	}

	mosquitto_username_pw_set(mosq, mqtt_username, mqtt_password);
	mosquitto_connect_callback_set(mosq, connect_callback);
	mosquitto_message_callback_set(mosq, message_callback);

	if (mosquitto_connect(mosq, mqtt_host, mqtt_port, 60)) {
		fprintf(stderr, "%s: unable to connect.\n", __FUNCTION__);
		exit(EXIT_FAILURE);
	}

	mosquitto_loop_forever(mosq, -1, 1);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
}

// <main>
int main(int argc, char *argv[]) {
	char connectionString[200];
	int timeout;
	char *tmp_value = NULL;
	openlog("Azure IoThub", 0, LOG_USER);

	uci = uci_alloc_context();
	tmp_value = ucix_get_option(uci, "azure_iothub", "azure_iothub", "connection_string");
	if(tmp_value != NULL){
		strncpy(connectionString, tmp_value, sizeof(connectionString));
		free(tmp_value);
		tmp_value = NULL;
	}else{
		connectionString[0] = 0;
	}
	tmp_value = ucix_get_option(uci, "azure_iothub", "azure_iothub", "message_interval");
	if(tmp_value != NULL){
		timeout = atoi(tmp_value);
		if(timeout <= 0)
			timeout = 300; //default 5min
		free(tmp_value);
		tmp_value = NULL;
	}else{
		timeout = 300; //default 5min
	}

	if (argv[1] && strcmp(argv[1], "-custom") == 0){
		if(argv[2] && argv[3]){
			strncpy(connectionString, argv[2], sizeof(connectionString));
		}else{
			syslog(LOG_INFO, "Missing connection string and socket file name arguments");
			exit(1);
		}
	}

	(void)printf("This sample simulates a Chiller device connected to the Remote Monitoring solution accelerator\r\n\r\n");
	if(strlen(connectionString) == 0){
		syslog(LOG_INFO, "Missing connection string!");
		exit(1);
	}
	// Used to initialize sdk subsystem
	(void)IoTHub_Init();
	syslog(LOG_INFO, "Creating IoTHub handle");
	// Create the iothub handle here
	device_handle = IoTHubDeviceClient_CreateFromConnectionString(connectionString, MQTT_Protocol);
	IoTHubDeviceClient_SetOption(device_handle, OPTION_TRUSTED_CERT, certificates);
	if (device_handle == NULL)
	{
		(void)printf("Failure creating Iothub device.  Hint: Check you connection string.\r\n");
		syslog(LOG_INFO, "Failure creating Iothub device.  Hint: Check you connection string");
	}else{
		// Setting connection status callback to get indication of connection to iothub
		(void)IoTHubDeviceClient_SetConnectionStatusCallback(device_handle, connection_status_callback, NULL);
		Chiller chiller;
		memset(&chiller, 0, sizeof(Chiller));
		chiller.protocol = "MQTT";
		chiller.telemetry.dynamicSchema.messageSchema.name = "dynamic-information;v1";
		chiller.telemetry.dynamicSchema.messageSchema.format = "JSON";

		sendChillerReportedProperties(&chiller);
		(void)IoTHubDeviceClient_SetDeviceMethodCallback(device_handle, device_method_callback, &chiller);
		if (argc == 1) {
			readUciConfigs();
			send_data(timeout, chiller.telemetry.dynamicSchema.messageSchema.name);
		} else if ((strcmp(argv[1], "-custom")) == 0) {
			open_server_socket(chiller.telemetry.dynamicSchema.messageSchema.name, argv[3]);
		} else if ((strcmp(argv[1], "-subscribe")) == 0) {
			subscriber_main();
		}
	}
	// Shutdown the sdk subsystem
	IoTHub_Deinit();

	return 0;
}
// </main>
