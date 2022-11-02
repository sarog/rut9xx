#pragma once

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <fcntl.h>
#include "libubus.h"
#include <libubox/blob.h>
#include <libubox/uloop.h>
#include <tlt_logger.h>

#include "system.h"
#include "utils.h"
#include "firewall.h"

#define BACKUP_INTERVAL	    600 // default interval value in seconds
#define DEF_ROTATE_INTERVAL 1440 //days
#define DEF_DATA_LIMIT	    10
#define MAX_IGNORE_LIST 8

#define MD_BUFFER_32   32
#define MD_BUFFER_64   64
#define MD_BUFFER_128  128
#define MD_BUFFER_256  256
#define MD_BUFFER_512  512
#define MD_BUFFER_1024 1024

#define MD_QUOTA_DIR "/proc/net/xt_quota/"

#define UNUSED(x) (void)(x)

enum md_vtype {
	MD_VAL_STRING,
	MD_VAL_INT,
};

struct config {
	/*User scripr to execute when interface goes up*/
	const char *ifup;
	/*User scripr to execute when interface goes down*/
	const char *ifdown;
	/*Path to pid file*/
	const char *pidfile;
	/*Log level*/
	log_level_type log_level;
	/*Log type*/
	unsigned int log_type;
	/*Data gathering interval*/
	unsigned int interval;
	/*Database backup interval*/
	int gzip_interval;
	/*Database rotation interval*/
	unsigned int rotate_interval;
	/*Percentage of prev collected bytes. Triggers database backup.*/
	unsigned int limit;
	/*Interfaces to ignore*/
	const char *ignore_list[MAX_IGNORE_LIST];
};

extern struct config g_conf;

void stop(void);