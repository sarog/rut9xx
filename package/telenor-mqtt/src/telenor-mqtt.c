#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <unistd.h>
#include "cJSON.h"
#include <stdint.h>
#include <libtlt_socket_man/socket_man.h>
#include "telenor-mqtt.h"
#include <libgsm/sms.h>
#include <libgsm/modem.h>
#include <libmnfinfo/mnfinfo.h>

static int run = -1;

void on_connect(struct mosquitto *mosq, void *obj, int rc) {
	syslog(LOG_INFO, "On connect: %d\n", rc);
	int mid;
	mosquitto_subscribe(mosq, &mid, MQTT_SUBSCRIBE, 0);
}

void on_disconnect(struct mosquitto *mosq, void *obj, int rc) {
	syslog(LOG_INFO, "On_disconnect");
	run = rc;
}

void log_callback(struct mosquitto *mosq, void *obj, int level, const char *str) {
	//syslog(LOG_INFO, "%s\n", str);
}

double get_timestamp(const struct mosquitto_message *message){
	double timestamp;
	cJSON *json = cJSON_Parse(message->payload);
	if (json == NULL) {
		printf("json null\n");
		return -1;
	}

	cJSON *stateObj = cJSON_GetObjectItem(json, "state");
	if (stateObj == NULL) {
		cJSON_Delete(json);
		printf("stateobj null\n");
		return -1;
	}

	cJSON *settingObj = cJSON_GetObjectItem(stateObj, MQTT_SET_VALUE);
	if (settingObj == NULL) {
		cJSON_Delete(json);
		printf("settingobj null\n");
		return -1;
	}
	else {
		if (settingObj->valueint == 1 || settingObj->valueint == 0) {
			cJSON *metadataObj = cJSON_GetObjectItem(json, "metadata");
			if (metadataObj == NULL) {
				cJSON_Delete(json);
				printf("metadataobj null\n");
				return -1;
			}
			cJSON *metadataValue = cJSON_GetObjectItem(metadataObj, MQTT_SET_VALUE);
			if (metadataValue == NULL) {
				cJSON_Delete(json);
				printf("metadatavalue null\n");
				return -1;
			}
			cJSON *metadata = cJSON_GetObjectItem(metadataValue, "timestamp");
			if (metadata == NULL) {
				cJSON_Delete(json);
				printf("metadata null\n");
				return -1;
			} else {
				timestamp = metadata->valuedouble;
				relay = settingObj->valueint;
				cJSON_Delete(json);
			}
		} else
			return -1;
	}
	return timestamp;
}

void set_output(const struct mosquitto_message *message, int start) {
	int execute = 0;
	double time;

	if (start == 0) {
		time = get_timestamp(message);
		if (time > MQTT_TIMESTAMP) {
			execute = 1;
			MQTT_TIMESTAMP = time;
		}
	}

	if (execute == 1 || start == 1) {
		if (relay == 1)
			system("/sbin/gpio.sh set DOUT2");
		else if (relay == 0)
			system("/sbin/gpio.sh clear DOUT2");
	}
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {
	//syslog(LOG_INFO, "Received message:\n%.*s\n\n", message->payloadlen, (char *)message->payload);
	modem_dev device = get_modem();
	if (MQTT_SET == 1 && counter > 0)
		set_output(message, 0);
	else if (MQTT_SET == 1 && counter == 0) {
		MQTT_TIMESTAMP = get_timestamp(message);
		set_output(NULL, 1);
		counter = 1;
	}
	char *temp = malloc(128);
	char *signal = malloc(128);
	char *wan = malloc(128);
	char *rel = malloc(128);
	char *imei = malloc(128);
	char *operator = malloc(128);
	char *serial = malloc(20);
	
	temp[0] = '\0';
	signal[0] = '\0';
	wan[0] = '\0';
	rel[0] = '\0';
	imei[0] = '\0';
	operator[0] = '\0';
	serial[0] = '\0';
	char *out;

	int socket_fd;
	int outputLength;
	int mid;
	int ck;

	socket_fd = open_socket();
	ck = gsmctl_get_temperature(socket_fd, device, &temp);
	temp[strlen(temp) - 1] = '\0';

	ck = gsmctl_get_signal_quality(socket_fd, device, (char **) &signal);
	signal[strlen(signal) - 1] = '\0';

	ck = gsmctl_get_ip("wwan0", &wan);

	ck = gsmctl_get_imei(socket_fd, device, &imei);
	imei[strlen(imei) - 1] = '\0';

	ck = gsmctl_get_operator(socket_fd, device, &operator);
	operator[strlen(operator) - 1] = '\0';
	close_socket(socket_fd);

	memset(serial, '\0', 20);
	mnfinfo_get_sn(&serial);

	if (MQTT_SET == 1) {
		FILE *po = popen("/sbin/gpio.sh get DOUT2", "r");
		fgets(rel, sizeof(rel), po);
		rel[strlen(rel) - 1] = '\0';
		pclose(po);
	}

	cJSON *root, *states, *reported;

	root = cJSON_CreateObject();
	states = cJSON_CreateObject();
	reported = cJSON_CreateObject();

	cJSON_AddItemToObject(root, "state", states);
	cJSON_AddStringToObject(states, "desired", "null");
	cJSON_AddStringToObject(reported, "temperature", temp);
	cJSON_AddStringToObject(reported, "signal", signal);
	cJSON_AddStringToObject(reported, "ip", wan);
	cJSON_AddStringToObject(reported, "imei", imei);
	cJSON_AddStringToObject(reported, "operator", operator);
	cJSON_AddStringToObject(reported, "serial", serial);
	
	if (MQTT_SET == 1)
		cJSON_AddStringToObject(reported, MQTT_SET_VALUE, rel);
	cJSON_AddItemToObject(states, "reported", reported);


	out = cJSON_Print(root);
	outputLength = strlen(out);
	//syslog(LOG_INFO, "Sending message: %s\n", out);

	cJSON_Delete(root);

	mosquitto_publish(mosq, &mid, MQTT_PUBLISH, outputLength, out, 0, false);
	mosquitto_publish(mosq, &mid, MQTT_PUBLISH, 36, "{\"state\":{\"desired\":{\"counter\":3}}}", 0, false);
	
	free(wan);
	free(signal);
	free(temp);
	free(out);
	free(rel);
	free(imei);
	free(operator);
	free(serial);
	sleep(MQTT_SLEEP);
}

void my_subscribe_callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos) {
	mosquitto_publish(mosq, &mid, MQTT_PUBLISH, 36, "{\"state\":{\"desired\":{\"counter\":2}}}", 0, false);
}

void readUciConfigs() {
	USE_CERT = 0;
	int certs = 0;

	uci = uci_init();
	char *data = NULL;

	data = ucix_get_option(uci, "telenor_mqtt", "telenor_mqtt", "telenor_ip");
	memset(MQTT_HOST, '\0', 100);
	strncpy(MQTT_HOST, data, 99);

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "telenor_mqtt", "telenor_mqtt", "telenor_port");
	MQTT_PORT = atoi(data);

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "telenor_mqtt", "telenor_mqtt", "ca_file");
	memset(CA_FILE, '\0', 100);
	if (data != NULL && strcmp(data, "") != 0) {
		strncpy(CA_FILE, data, 99);
		certs++;
	}

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "telenor_mqtt", "telenor_mqtt", "cert_file");
	memset(CERT_FILE, '\0', 100);
	if (data != NULL && strcmp(data, "") != 0) {
		strncpy(CERT_FILE, data, 99);
		certs++;
	}

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "telenor_mqtt", "telenor_mqtt", "key_file");
	memset(KEY_FILE, '\0', 100);
	if (data != NULL && strcmp(data, "") != 0) {
		strncpy(KEY_FILE, data, 99);
		certs++;
	}

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "telenor_mqtt", "telenor_mqtt", "telenor_id");
	memset(MQTT_ID, '\0', 15);
	strncpy(MQTT_ID, data, 14);

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "telenor_mqtt", "telenor_mqtt", "sleep");
	if (data != NULL)
		MQTT_SLEEP = atoi(data);

	if (data)
		free(data);
	data = NULL;

	data = ucix_get_option(uci, "telenor_mqtt", "telenor_mqtt", "enable_setting");
	if (data != NULL)
		MQTT_SET = atoi(data);

	if (data)
		free(data);
	data = NULL;

	if (MQTT_SET == 1){
		data = ucix_get_option(uci, "telenor_mqtt", "telenor_mqtt", "setting_value");
		memset(MQTT_SET_VALUE, '\0', 15);
		strncpy(MQTT_SET_VALUE, data, 14);
		if (data)
			free(data);
		data = NULL;
	}

	if (certs == 3)
		USE_CERT = 1;
}

void createThingAddress(){
	snprintf(MQTT_SUBSCRIBE, sizeof(MQTT_SUBSCRIBE), "$aws/things/%s/shadow/update/delta", MQTT_ID);
	snprintf(MQTT_PUBLISH, sizeof(MQTT_SUBSCRIBE), "$aws/things/%s/shadow/update", MQTT_ID);
}

int main(int argc, char *argv[]) {
	int rc;
	struct mosquitto *mosq;
	readUciConfigs();
	createThingAddress();
	mosquitto_lib_init();
	openlog("telenor-mqtt", 0, LOG_USER);

	mosq = mosquitto_new(MQTT_ID, true, NULL);
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_disconnect_callback_set(mosq, on_disconnect);
	mosquitto_log_callback_set(mosq, log_callback);
	mosquitto_message_callback_set(mosq, message_callback);
	mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
	
	if (mosq)
		syslog(LOG_INFO, "Connected to MQTT");
	else
		syslog(LOG_ERR, "Connect to MQTT failed");
	if (mosquitto_tls_set(mosq, CA_FILE, NULL, CERT_FILE, KEY_FILE, NULL)) {
		syslog(LOG_ERR, "Problem setting TLS certificates");
		return 1;
	} else
		syslog(LOG_INFO, "TLS certificates set");
	if (mosquitto_tls_opts_set(mosq, 1, "tlsv1.2", NULL)) {
		syslog(LOG_ERR, "Problem setting TLS opts");
		return 1;
	}
	if (mosquitto_tls_insecure_set(mosq, true)) {
		syslog(LOG_ERR, "Problem setting TLS insecure");
		return 1;
	}

	if (mosquitto_connect_bind(mosq, MQTT_HOST, MQTT_PORT, 60, 0)) {
		syslog(LOG_ERR, "Unable to connect to cloud system");
		return 1;
	} else
		syslog(LOG_INFO, "Connected to cloud system");

	mosquitto_loop_forever(mosq, -1, 1);

	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	
	return rc;
}
