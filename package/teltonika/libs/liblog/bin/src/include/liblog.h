#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include <libubus.h>

#define LLOGD_UBUS_OBJECT     "log"
#define LLOGD_ADD_TASK_METHOD "write_ext"
#define LLOGD_READ_DB_METHOD  "read_db"
#define BUFFER_SIZE           128

#define ID_VALUE       "id"
#define TIME_VALUE     "time"
#define SOURCE_VALUE   "source"
#define PRIORITY_VALUE "priority"
#define TEXT_VALUE     "text"

typedef enum { LLOG_SUCCESS, LLOG_ERROR } llog_status_t;

typedef struct _code {
	char *c_name;
	int c_val;
} CODE;
/**
 * @brief Event log tables
 */
typedef enum {
	LLOG_EVENTS,     /** Regular events */
	LLOG_SYSTEM,     /** System events */
	LLOG_NETWORK,    /** Network events */
	LLOG_CONNECTIONS /** Network connection events */
} log_table;

/**
 * @brief Event priority numbers
 */
typedef enum {
	EVENTLOG_EMERG,
	EVENTLOG_ALERT,
	EVENTLOG_CRIT,
	EVENTLOG_ERR,
	EVENTLOG_WARNING,
	EVENTLOG_NOTICE,
	EVENTLOG_INFO,
	EVENTLOG_DEBUG
} event_priority;

extern CODE log_facility_names[];

/**
 * @brief Event structure
 */
struct events_log {
	int db_flag; /** 0 - write to log db, 1 - write to events log db */
	log_table table;
	event_priority priority;
	char *text;
	char *sender;
};
/**
 * Read log messages. Invokes ubus "read_db" procedure
 * @param ubus - UBUS context
 * @param table -	table id
 * @param output -	Pointer to a struct blob_attr binary buffer that contains the result.
 * Refer to the example program on how to parse this.
 * @return -	0 on success, -1 on failure
 */
llog_status_t llog_read_tasks(struct ubus_context *ubus, log_table table, struct blob_attr **output);

/**
 * Read log messages, extended version. Invokes ubus "read_db" procedure
 * @param ubus - UBUS context
 * @param table - table id
 * @param query - Part of the SQL query (select * from table name <query>) e.g. "WHERE priority=1"
 * @param output -	Pointer to a struct blob_attr binary buffer that contains the result.
 * Refer to the example program on how to parse this.
 * @return - 0 on success, -1 on failure
 */
llog_status_t llog_read_tasks_ex(struct ubus_context *ubus, log_table table, const char *query,
                                 struct blob_attr **output);

/**
 *	Add event to database in form of the task
 * @param ubus - UBUS context
 * @param new_task - event structure
 * @return - 0 on success, -1 on failure
 */
llog_status_t llog_add_task(struct ubus_context *ubus, struct events_log *new_task);

/**
 * Form task and add event to data base
 * @param ubus - UBUS context
 * @param priority - message priority
 * @param sender - identification string e.g. "SIM Switch"
 * @param text - event text
 * @return - 0 on success, -1 on failure
 */
llog_status_t llog_insert_event_into_db(struct ubus_context *ubus, event_priority priority, char *sender,
                                        char *text);

#endif
