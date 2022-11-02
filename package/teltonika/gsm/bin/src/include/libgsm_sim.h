#pragma once

#include <json-c/json.h>
#include <libubus.h>
#include <libubox/blobmsg_json.h>
#include <libubox/blobmsg.h>
#include <libubox/blob.h>
#include <libgsm_modem.h>
#include <libsim.h>

void get_sim_output_callback(struct ubus_request *req, int type, struct blob_attr *msg);
int gsmctl_sim_sw_ubus(int *output, char *modem_id);
int gsmctl_sim_get_ubus(int *output, char *modem_id);
int gsmctl_set_modem_sim(modem_dev device, char *output, char *modem_id);
