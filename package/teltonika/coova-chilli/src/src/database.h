//
// Created by darius on 19.5.23.
//

#ifndef RUTX_DATABASE_H
#define RUTX_DATABASE_H

#define DB_SUCCESS 0
#define DB_FAIL 1


#define TABLE_NAME "statistics"
#define INSERT_FMT "INSERT INTO "TABLE_NAME" (start_time, mac, ip, username, session, ifname, sessionid," \
        "authmode) VALUES (%ld, '"MAC_FMT"', '%s', '%s', %d, '%s', '%s', '%d');"
#define UPDATE_FMT "UPDATE "TABLE_NAME \
                 " SET last_update = %ld, idletime = %ld, sessiontime = %ld, input_octets = %lld," \
                 " output_octets = %lld, input_packets = %lld, output_packets = %lld, session = %d," \
                 " terminate_cause = %d" \
                 " WHERE mac = '"MAC_FMT"' AND session = %d AND sessionid = '%s';"

struct db_session_state {
    uint64_t input_octets;
    uint64_t output_octets;
    uint64_t input_packets;
    uint64_t output_packets;
    uint32_t sessiontime;
};

sqlite3 *dbopen(void);
int dbclose(sqlite3 *db);
int dbexec(sqlite3 *db, char *sql, int (*callback)(void*,int,char**,char**));
int dbcreate_table(sqlite3 *db);
int dbwrite(sqlite3 *db, struct app_conn_t *appconn);
int dbupdate(sqlite3 *db, struct app_conn_t *appconn);
int dbcheck_table(void);
int dbconup(struct app_conn_t *conn);
int dbconupdate(struct app_conn_t *conn);
int dbsession_state(struct app_conn_t *conn);
int dbcheck_session(struct redir_conn_t *conn);

#endif //RUTX_DATABASE_H
