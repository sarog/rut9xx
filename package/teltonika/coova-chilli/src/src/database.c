#include "chilli.h"

static int _seletc_callback(void *str_session, int argc, char **argv, char **azColName) {
    struct db_session_state *p_str_session = (struct db_session_state *)str_session;

    if (argc > 0) {
        p_str_session->input_packets = argv[0] ? atoll(argv[0]) : 0;
        p_str_session->output_packets = argv[1] ? atoll(argv[1]) : 0;
        p_str_session->input_octets = argv[2] ? atoll(argv[2]) : 0;
        p_str_session->output_octets = argv[3] ? atoll(argv[3]) : 0;
        p_str_session->sessiontime = argv[4] ? atoi(argv[4]) : 0;

    }

    return DB_SUCCESS;
}

static int dbsession_data(struct session_params *s_params, char *username, uint8_t *hismac,
        struct db_session_state * db_state){
    int ret;
    char *sql;
    char *err = 0;
    sqlite3 *db;
    time_t timestamp_now = mainclock_wall();
    struct tm *time_now;
    int start_wday, wday;

    if (_options.debug)
        syslog(LOG_INFO, "[%s] Trying to get sesions state %ld", __FUNCTION__, (long) timestamp_now);

    time_now = localtime(&timestamp_now);
    time_now->tm_min = 0;
    time_now->tm_sec = 0;

    switch (s_params->period)
    {
        case PERIOD_MONTH:
            if (s_params->start > time_now->tm_mday)
                time_now->tm_mon--;

            time_now->tm_mday = s_params->start;
            time_now->tm_hour = 0;

            break;
        case PERIOD_WEEK:
            start_wday = s_params->start == 0 ? 7 : s_params->start;
            wday = time_now->tm_wday == 0 ? 7 : time_now->tm_wday;

            if (time_now->tm_wday != s_params->start){
                if (start_wday > wday)
                    time_now->tm_mday -= (7 - start_wday - wday);
                else
                    time_now->tm_mday -= wday -start_wday;
            }

            time_now->tm_hour = 0;

            break;
        case PERIOD_DAY:
            if (s_params->start > time_now->tm_hour)
                time_now->tm_mday--;

            time_now->tm_hour = s_params->start;

            break;
    }

    db = dbopen();
    asprintf(&sql, "SELECT SUM(input_packets) AS input_packets, SUM(output_packets) AS output_packets," \
                   "SUM(input_octets) AS input_octets, SUM(output_octets) AS output_octets," \
                   " SUM(sessiontime) AS sessiontime FROM "TABLE_NAME \
                   " WHERE start_time >= %ld AND username = '%s';", (long) mktime(time_now), username);
    if (_options.debug)
        syslog(LOG_INFO, "[%s] SQL: %s", __FUNCTION__ , sql);

    ret = sqlite3_exec(db, sql, _seletc_callback, db_state, &err);
    if (ret){
        syslog(LOG_INFO, "[%s] SQL error: %s\n", __FUNCTION__, err);
        sqlite3_free(err);
    }

    dbclose(db);
    free(sql);

    return ret;
}

sqlite3 *dbopen(void) {
    sqlite3 *db;
    int ret;

    ret = sqlite3_open(_options.dbpath, &db);
    if (ret)
        syslog(LOG_INFO, "[%s] Can't open database: %s", __FUNCTION__ , sqlite3_errmsg(db));

    return db;
}

int dbclose(sqlite3 *db){
    return sqlite3_close(db);
}

int dbexec(sqlite3 *db, char *sql, int (*callback)(void*,int,char**,char**)){
    int ret;
    char *err = 0;

    ret = sqlite3_exec(db, sql, callback, 0, &err);
    if (ret){
        syslog(LOG_INFO, "[%s] SQL error: %s\n", __FUNCTION__, err);
        sqlite3_free(err);
    }

    return ret;
}

int dbprepare(sqlite3 *db, char *sql){
    sqlite3_stmt *stmt;
    int ret;

    ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (ret) {
        syslog(LOG_INFO, "SQL error: %s\n", sqlite3_errmsg(db));
        goto out;
    }

    ret = sqlite3_step(stmt);
    if (ret){
        syslog(LOG_INFO, "SQL error: %s\n", sqlite3_errmsg(db));
        goto out;
    }

    out:
    return ret;
}

int dbcreate_table(sqlite3 *db){
    int ret;
    char *sql;

    sql = "CREATE TABLE "TABLE_NAME"("
      "id               INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE," \
      "start_time       TIMESTAMP NOT NULL," \
      "last_update      TIMESTAMP," \
      "mac              VARCHAR(32)," \
      "ip               VARCHAR(16)," \
      "username         VARCHAR(64)," \
      "sessiontime      BIGINT NOT NULL DEFAULT 0," \
      "idletime         BIGINT NOT NULL DEFAULT 0," \
      "input_octets     BIGINT NOT NULL DEFAULT 0," \
      "output_octets    BIGINT NOT NULL DEFAULT 0," \
      "input_packets    BIGINT NOT NULL DEFAULT 0," \
      "output_packets   BIGINT NOT NULL DEFAULT 0," \
      "session          BOOLEAN NOT NULL DEFAULT 0," \
      "ifname           VARCHAR(16)," \
      "sessionid        VARCHAR(33)," \
      "terminate_cause  BIGINT NOT NULL DEFAULT 0," \
      "custom           VARCHAR(64)," \
      "authmode         VARCHAR(64));";
    ret = dbexec(db, sql, NULL);

    return ret;
}

int dbwrite(sqlite3 *db, struct app_conn_t *conn){
    int ret;
    char *sql = NULL;
    time_t time_now;
    struct session_state s_state;

    time_now = mainclock_wall();
    s_state = conn->s_state;

    asprintf(&sql, INSERT_FMT, (long int)time_now, MAC_ARG(conn->hismac),
             inet_ntoa(conn->hisip), s_state.redir.username, s_state.authenticated,
#ifdef ENABLE_MULTILAN
			 app_conn_idx(conn) ? _options.moreif[app_conn_idx(conn)-1].dhcpif : _options.dhcpif,
#else
			 _options.dhcpif,
#endif
             s_state.sessionid, conn->s_state.redir.auth_mode);
    if (_options.debug)
        syslog(LOG_INFO, "[%s] SQL: %s", __FUNCTION__ , sql);

    ret = dbexec(db, sql, NULL);

    free(sql);
    return ret;
}

int dbupdate(sqlite3 *db, struct app_conn_t *conn){
    int ret;
    char *sql = NULL;
    uint32_t time_now, idletime, sessiontime;
    struct session_state s_state;

    s_state = conn->s_state;
    time_now = mainclock_wall();
    idletime = mainclock_diffu(s_state.last_up_time);
    sessiontime = mainclock_diffu(s_state.start_time);
    asprintf(&sql, UPDATE_FMT, (long int)time_now, (long int)idletime, (long int)sessiontime,
             s_state.input_octets, s_state.output_octets, s_state.input_packets,
             s_state.output_packets, s_state.authenticated, s_state.terminate_cause_ui,
			 MAC_ARG(conn->hismac), 1, s_state.sessionid);

    if (_options.debug)
        syslog(LOG_INFO, "[%s] SQL: %s", __FUNCTION__ , sql);
    ret = dbexec(db, sql, NULL);
    free(sql);

    return ret;
}

int dbtable_exists(sqlite3 *db){
    int ret;
    int count = 0;
    char *sql;
    sqlite3_stmt *stmt;

    sql = "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='"TABLE_NAME"';";
    ret = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (ret != SQLITE_OK) {
        syslog(LOG_INFO, "[%s] SQL error: %s", __FUNCTION__ , sqlite3_errmsg(db));
        return DB_FAIL;
    }

    if ((ret = sqlite3_step(stmt)) == SQLITE_ROW)
        count = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return (count == 0) ? DB_FAIL : DB_SUCCESS;
}

int dbcheck_table(void){
    sqlite3 *db;
    int ret = -1;

    db = dbopen();
    if (dbtable_exists(db) == DB_FAIL)
        ret = dbcreate_table(db);

    dbclose(db);

    return ret;
}

int dbconup(struct app_conn_t *conn){
    sqlite3 *db;
    int ret = -1;

    db = dbopen();
    if (db != NULL) {
        ret = dbwrite(db, conn);
        dbclose(db);
    }

    return ret;
}

int dbconupdate(struct app_conn_t *conn){
    sqlite3 *db;
    int ret = -1;

    db = dbopen();
    if (db) {
        ret = dbupdate(db, conn);
        dbclose(db);
    }

    return ret;
}

int dbcheck_session(struct redir_conn_t *conn){
    int ret = ACCESS_ACCEPTED;
    uint32_t sessiontime, start_time;
    struct db_session_state db_state;

    if (strlen(conn->s_state.redir.username) == 0)
        return 0;

    dbsession_data(&conn->s_params, conn->s_state.redir.username, conn->hismac, &db_state);

    if (_options.debug)
        syslog(LOG_INFO, "[%s] input_octets %lld, output_octets %lld,  sessiontime %d", __FUNCTION__,
               db_state.input_octets, db_state.output_octets, db_state.sessiontime);

    if (conn->s_params.maxinputoctets &&
        conn->s_params.maxinputoctets < db_state.input_octets)
        ret = ACCESS_DENIED_DATA;
    else if (conn->s_params.maxoutputoctets &&
             conn->s_params.maxoutputoctets < db_state.output_octets)
        ret = ACCESS_DENIED_DATA;
    else if (conn->s_params.sessiontimeout) {
        start_time = mainclock_now() - db_state.sessiontime;
        sessiontime = mainclock_diffu(start_time);
        if (conn->s_params.sessiontimeout < sessiontime)
            ret = ACCESS_DENIED_TIME;
    }

    return ret;
}

int dbsession_state(struct app_conn_t *conn){
    int ret;
    struct db_session_state db_state;

    if (strlen(conn->s_state.redir.username) == 0)
        return 0;

    ret = dbsession_data(&conn->s_params, conn->s_state.redir.username, conn->hismac, &db_state);

    if (!ret){
        if (_options.debug)
        syslog(LOG_INFO, "[%s] input_octets %lld, output_octets %lld,  sessiontime %d", __FUNCTION__,
               db_state.input_octets, db_state.output_octets, db_state.sessiontime);
        conn->s_history.input_packets = db_state.input_packets;
        conn->s_history.output_packets = db_state.output_packets;
        conn->s_history.input_octets = db_state.input_octets;
        conn->s_history.output_octets = db_state.output_octets;
        conn->s_history.sessiontime = db_state.sessiontime;
    }

    return ret;
}
