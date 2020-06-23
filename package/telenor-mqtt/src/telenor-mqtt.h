#ifndef MQTT_PUB_H
#define MQTT_PUB_H

#include <stdbool.h>

void readUciConfigs();
double get_timestamp(const struct mosquitto_message *message);
void set_output(const struct mosquitto_message *message, int start);
int USE_CERT;
char MQTT_SUBSCRIBE[45];
char MQTT_PUBLISH[45];
char MQTT_HOST[100];
int MQTT_PORT;
char CA_FILE[100];
char CERT_FILE[100];
char KEY_FILE[100];
char MQTT_ID[15];
int MQTT_SLEEP;
int MQTT_SET;
char MQTT_SET_VALUE[15];
double MQTT_TIMESTAMP;
int counter = 0;
int relay = 2;
int DEBUG_LEVEL;
struct uci_context *uci;

#endif 
