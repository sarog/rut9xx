//
// Created by simonas on 16.5.19.
//
/*****************************************
 * 	Remastered
 *****************************************/
#ifndef UNHANDLER_H
#define UNHANDLER_H

#include "lock.h"
#include "getpid.h"

#define EVENTS_PKG "events_reporting"
#define BLOCK_FILE "/tmp/events_reporting_block"
#define PREV_SIG_FILE "/tmp/events_reporting_prev"
#define EVENT_MSG "Signal strength dropped below %d dBm"
#define EVENT_MSG_MATCH "Signal strength %*s below %d dBm"

/*****************************************
 * 	main unsolicited functions
 *****************************************/
void unsolicited_simstate_keeper(char *arg);
void unsolicited_simstate_keeper_huawei(char *arg);
void unsolicited_netstate_keeper(char *arg);
void unsolicited_netstate_keeper_huawei(char *arg);
void unsolicited_conntype_keeper(char *arg);
void unsolicited_conntype_keeper_huawei(char *arg);
void unsolicited_conntype_keeper_sierra(char *arg);
void SMS_script(char *arg);
void write_output_to_file(char *text);

void Calling_telit(char *arg);
int interface_hotplug_event(char *arg);
void check_connection_type(char *arg);
void log_connection_event(char *event);
void Roaming(char *arg);
void unhandler(const char *argumentas);
char *get_interface_ip(char * interface);

/**********
 * helper.h
 **********/
void insert_into_events(char *tabl, char *typ, char *txt);
char *get_operator_from_socket(void);

int save_state(char *name, char *value, char *file);

/**********
 * signal_str.h
 **********/
void check_the_rules(int strength);
void signal_script(char *arg);
void signal_huawei(char *arg);
void signal_quectel(char *arg);
void signal_telit(char *arg);

/**********
 * operator.h
 **********/
void write_operator_to_file(const char *operator);
void read_operator_from_file(char *operator, int len);
void operator_log(int renew);

#endif
