#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <uci.h>

#define REMOVE 0
#define ADD 1
#define CHECK 2

#define BADKEY -1
//FIXME: Use enum instead
#define GPS					1
#define RS485				2
#define CLI					3
#define DROPBEAR			4
#define RS232				5
#define MULTIWAN			6
#define PING_REBOOT			7
#define SAMBA				8
#define SIM_SWITCH			9
#define PRIVOXY				10
#define EVENTSLOG_REPORT	11
#define STRONGSWAN			12
#define SNMPD				13
#define REREGISTER			14
#define PPTPD				15
#define OUTPUT_CONTROL		16
#define SMPP_INIT			17
#define LOGTRIGGER			18
#define SIM_IDLE_PROTECTION	19
#define VRRPD				20
#define SIMPIN				21
#define RADIUS				22
#define OPENVPN_VPN			23
#define MDCOLLECTD			24
#define SMSCOLLECT			25
#define HOSTBLOCK			26
#define LIMIT_GUARD			27
#define TCP_LOGGER			28
#define EASYCWMP			29
#define PERIODIC_REBOOT		30
#define SMS_GATEWAY			31
#define DHCP_RELAY			32
#define MOSQUITTO			33
#define MQTT_PUB			34
#define MODBUS				35
#define RMS_CONNECT			36
typedef struct { char *key; int val; } t_symstruct;

int checksv(int argc, char *argv[]);

int add_remove_symlink(char *init_d_name, int action);
