
#include "sqlite.h"
#include "system.h"

sqlite3 *sqlopen(char *dbpath) {
    sqlite3 *db;
    int ret;

    ret = sqlite3_open(dbpath, &db);
    if (ret)
        syslog(LOG_INFO, "[%s] Can't open database: %s", __FUNCTION__ , sqlite3_errmsg(db));

    return db;
}

int sqlclose(sqlite3 *db){
    return sqlite3_close(db);
}

int sqlexec(sqlite3 *db, char *sql, int (*callback)(void*,int,char**,char**)){
    int ret;
    char *err = 0;

    ret = sqlite3_exec(db, sql, callback, 0, &err);
    if (ret){
        syslog(LOG_INFO, "[%s] SQL error: %s\n", __FUNCTION__, err);
        sqlite3_free(err);
    }

    return ret;
}

int sqlprepare(sqlite3 *db, char *sql){
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

int sqltable_exists(sqlite3 *db, char *table_name){
    int ret;
    int count = 0;
    char *sql = NULL;
    sqlite3_stmt *stmt;

    asprintf(&sql, "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='%s';",
			 table_name);
    ret = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (ret != SQLITE_OK) {
        syslog(LOG_INFO, "[%s] SQL error: %s", __FUNCTION__ , sqlite3_errmsg(db));
        return SQL_FAIL;
    }

    if ((ret = sqlite3_step(stmt)) == SQLITE_ROW)
        count = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return (count == 0) ? SQL_FAIL : SQL_SUCCESS;
}
