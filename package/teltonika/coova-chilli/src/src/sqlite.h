//
// Created by darius on 19.5.23.
//

#ifndef RUTX_SQLITE_H
#define RUTX_SQLITE_H

#include <sqlite3.h>

#define SQL_SUCCESS 0
#define SQL_FAIL 1

sqlite3 *sqlopen(char *dbpath);
int sqlclose(sqlite3 *db);
int sqlexec(sqlite3 *db, char *sql, int (*callback)(void*,int,char**,char**));
int sqlprepare(sqlite3 *db, char *sql);
int sqltable_exists(sqlite3 *db, char *table_name);

#endif //RUTX_SQLITE_H
