#pragma once

#include <time.h>
#include <libsim.h>
#include <sqlite3.h>

#include "system.h"

#define APPNAME "mdcollectd"

/* Path variables */
#define DATAPATH "/usr/lib/" // prefix for database files
#define SAVEPATH DATAPATH "" APPNAME "/" // place to store database files

/* Database variables */
#define DBPATH	    "/var/" // place for running database
#define DBNAME	    APPNAME ".db" // database file name
#define DBNAMEC	    APPNAME "1.db" // database file name
#define DBFILE	    DBNAME "_new.gz"
#define DBFULLPATH  DBPATH "" DBNAME // full path to database file
#define DBFULLPATHC DBPATH "" DBNAMEC // full path to database file
#define DBSAVEPATH                                                             \
	SAVEPATH "" DBNAME // full path to save compressed database files

#define DB_T_CURRENT "current"
#define DB_T_DAYS    "days"

#define DB_2_DAYS		172800
#define DB_SQLITE3_BUSY_TIMEOUT 1000

#define DB_CREATE_FMT                                                                                          \
	"CREATE TABLE IF NOT EXISTS %s (id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE, time TIMESTAMP, sim INT," \
	" modem VARCHAR(50), interface VARCHAR(50), rx INT, tx INT, name VARCHAR(50))"
#define DB_COUNT_TABLES_FMT                                                    \
	"SELECT count(*) FROM sqlite_master WHERE type='table' AND name='%s';"
#define DB_COND_FMT                                                            \
	"interface = '%s' AND time = '%ld' AND sim = '%d' AND modem = '%s' AND name = '%s'"
#define DB_INS_REP_FMT                                                         \
	"INSERT OR REPLACE INTO " DB_T_CURRENT                                 \
	" (id, time, sim, modem, interface, rx, tx, name) VALUES"                    \
	" ((SELECT id FROM " DB_T_CURRENT " WHERE %s),"                        \
	" '%ld', %d, '%s', '%s',"                                              \
	" ifnull((SELECT ifnull(rx, 0) FROM " DB_T_CURRENT                     \
	" WHERE %s), 0) + %lld,"                                               \
	" ifnull((SELECT ifnull(tx, 0) FROM " DB_T_CURRENT                     \
	" WHERE %s), 0) + %lld, '%s');"
#define DB_ROTATE_DAYS                                                         \
	"DELETE FROM " DB_T_DAYS " WHERE time < strftime('%%s','%s');"
#define DB_DEL_DAYS                                                            \
	"DELETE FROM " DB_T_DAYS                                               \
	" WHERE time = strftime('%%s','%s') AND sim = '%d' AND modem = '%s'" \
	" AND interface = '%s' AND name ='%s';"
#define DB_SUM_TO_DAYS                                                                              \
	"INSERT INTO " DB_T_DAYS " (time, sim, modem, interface, rx, tx, name)"                           \
	" SELECT strftime('%%s','%s') AS time, sim, modem, interface, SUM(rx) AS rx, SUM(tx) AS tx, name" \
	" FROM " DB_T_CURRENT                                                                       \
	" WHERE time >= '%ld' AND time <= '%ld' AND sim = '%d' AND modem ='%s' AND interface = '%s' AND name = '%s';"

extern sqlite3 *g_db;

typedef struct {
	int year;
	int month;
	int day;
	time_t stime;
} Date;

struct db_iface_data {
	Date date;
	const char *lo_iface;
	char *l3_device;
	char *modem_name;
	lsim_t sim;
	unsigned long long rx;
	unsigned long long tx;
};

int md_create_table_BD(char *query_fmt, char *table);
md_status_t sanitise_db(const char *ifname);
md_status_t backup_db(void);
sqlite3 *md_init_db();
void close_DB(void);
md_status_t md_restore_db(void);
int update_db();

