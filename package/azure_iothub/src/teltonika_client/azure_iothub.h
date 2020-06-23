#ifndef RUT200_WRT_AZURE_IOTHUB_H
#define RUT200_WRT_AZURE_IOTHUB_H

void readUciConfigs();
char ipaddr[2];
char bsent[2];
char brecv[2];
char connstate[2];
char netstate[2];
char imei[2];
char iccid[2];
char model[2];
char manuf[2];
char serial[2];
char revision[2];
char imsi[2];
char simstate[2];
char pinstate[2];
char gsm_signal[2];
char rscp[2];
char ecio[2];
char rsrp[2];
char sinr[2];
char rsrq[2];
char cellid[2];
char operator[2];
char opernum[2];
char conntype[2];
char temp[2];
char pincount[2];

char topic[40];
char mqtt_host[40];
int mqtt_port;
char mqtt_username[20];
char mqtt_password[20];

struct uci_context *uci;

#endif //RUT200_WRT_AZURE_IOTHUB_H
