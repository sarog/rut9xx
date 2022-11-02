#pragma once

#include <stdint.h>
#include <libubus.h>

#include "db.h"

#define IFACE_FLAG_NONE	  0x01
#define IFACE_FLAG_UPDATE 0x02

#define UBUS_INVOKE_TIMEOUT 10

#define SIZE_MODEM_NAME 16
#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#define UBUS_NET_OBJECT	 "network.interface"
#define UBUS_DUMP_METHOD "dump"

typedef enum {
	IF_TYPE_MOBILE,
	IF_TYPE_WIRED,
	IF_TYPE_WIFI,
} md_iftype;

typedef enum { MD_IFUP, MD_IFDOWN } md_ifstate;

struct interface_throughput {
	unsigned long long rx_prev;
	unsigned long long tx_prev;
	unsigned long long prev_total_rx;
	unsigned long long prev_total_tx;
	unsigned long long total_tx;
	unsigned long long total_rx;
};

struct md_interface_node {
	struct list_head list;
	/* Logical interface name */
	char *name;
	/*Interface type*/
	md_iftype type;
	/*SIM id if interface type is mobile*/
	lsim_t sim;
	/*Interace status */
	md_ifstate status;
	/*Control flag*/
	int flag;
	/*Throughput data*/
	struct interface_throughput data;
	/*Uloop timer*/
	struct uloop_timeout throughput_timeout;
	/*Interface check interval*/
	int checkup_interval;
	/*Function to hanle interface up event*/
	md_status_t (*ifup)(struct md_interface_node *interface);
	/*Function to hanle interface down event*/
	md_status_t (*ifdown)(struct md_interface_node *interface);
	/*Interface name*/
	char l3_device[IFNAMSIZ];
	/*Modem id if interface type is mobile*/
	char modem_name[SIZE_MODEM_NAME];
};

typedef int (*interface_cb_t)(struct md_interface_node *iface, void *priv);

void alloc_interface(struct blob_attr *msg);
/**
 * Removes interface from the list
 * @param interface pointer to interface structure
 */
void remove_interface(struct md_interface_node *interface);
int interface_ubus_request(struct ubus_context *ctx, char *v_iface,
			   const char *modem_id, int iptype);
void foreach_interface(interface_cb_t cb, void *priv);
struct md_interface_node *find_interface(const char *ifname);
void add_db_update_timer(struct uloop_timeout *t, unsigned int delay);
void db_update_timer(struct uloop_timeout *t);
md_status_t init_interfaces(struct ubus_context *ctx);
void set_db_data(struct db_iface_data *db_data,
		 struct md_interface_node *interface);
void proccess_throughput(struct md_interface_node *interface,
			 unsigned long long tx, unsigned long long rx);
