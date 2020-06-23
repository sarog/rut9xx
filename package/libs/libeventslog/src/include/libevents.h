#ifndef EVENT_LIB_H
#define EVENT_LIB_H
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
//#include <sqlite3.h>

#include <time.h> 

#include <sys/types.h>          /* See NOTES */
#include <sys/un.h>

#define LOG_UNIX_SOCK_PATH "/tmp/event.sock"
#define DB "/log/log.db"
#define PRINT_DB 1
#define INSERT 2

struct eventslog{
	int requests;
	int date;
	int limit;
	char *table;
	char *query;
	char *type;
	char *text;
	char *order;
	char *file;
	char *file_end;
};

int print_events_log_db(struct eventslog *new_task);
void default_task(struct eventslog *new_task);
void test_test();
int execute_task(struct eventslog *new_task);
int insert_events_log_db(struct eventslog *new_task);
void insert_event_into_db(char *tabl, char *typ, char *txt);
#endif
