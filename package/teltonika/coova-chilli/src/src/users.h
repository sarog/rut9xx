
#ifndef RUTX_USERS_H
#define RUTX_USERS_H

#define USER_MIN_PHONE_LEN 6

#define USER_USERNAMENAMESIZE 128
#define USER_PASSWORDSIZE 128
#define USER_PHONESIZE 64
#define USER_EMAILSIZE 128

#define USER_RAND_USERNAME_LEN 16
#define USER_RAND_PASSWORD_LEN 6

#define USER_RET_SUCCESS 0
#define USER_RET_ERROR 1
#define USER_RET_ALREADY 2
#define USER_RET_SMS_ERR 3

#define USER_MAX_MESSAGE_SIZE 128

enum {
	COL_SMS_USER_ID = 0,
	COL_SMS_USER_NAME,
	COL_SMS_USER_PASS,
	COL_SMS_USER_MAC,
	COL_SMS_USER_PHONE,
	COL_SMS_USER_CREATED,
	COL_SMS_USER_EXPIRATION,
	COL_SMS_USER_TIME,
};

enum {
	COL_USER_ID = 0,
	COL_USER_NAME,
	COL_USER_EMAIL,
	COL_USER_PASS,
	COL_USER_MAC,
	COL_USER_PHONE,
	COL_USER_CREATED,
	COL_USER_EXPIRATION,
	COL_USER_TIME,
};


#define INSERT_SMS_USERS_FMT "INSERT INTO sms_users (username, password, mac, phone," \
			" expiration) VALUES ('%s', '%s', '"MAC_FMT"', '%s', '%lld');"

#define NOT_EXPIRED_FMT "((strftime('%%s','now') - strftime('%%s', created)) < expiration" \
			" OR expiration = '0')"

#define LIMIT_FMT " ORDER BY id DESC limit 1"

#define USER_TIME_FMT "(strftime('%%s','now') - strftime('%%s', created)) AS user_time"

//Cont all unexpired users with corresponding phone number
#define SELECT_NOT_EXPIRED_FMT "SELECT COUNT(*) from %s" \
			" WHERE ("NOT_EXPIRED_FMT" AND (phone = '%s' OR mac = '"MAC_FMT"'));"

//Cont all unexpired users with corresponding mac
#define SELECT_NOT_EXPIRED_MAC_FMT "SELECT COUNT(*) from %s" \
			" WHERE ("NOT_EXPIRED_FMT" AND mac = '"MAC_FMT"');"

#define SELECT_NOT_EXPIRED_EMAIL_FMT "SELECT COUNT(*) from %s" \
			" WHERE ("NOT_EXPIRED_FMT" AND (email = '%s' OR mac = '"MAC_FMT"'));"

#define SELECT_SMS_USER_FMT "SELECT *, "USER_TIME_FMT"  FROM sms_users" \
			" WHERE ("NOT_EXPIRED_FMT" AND password = '%s')"LIMIT_FMT";"


#define INSERT_USERS_FMT "INSERT INTO users (username, email, password, mac, phone," \
			" expiration) VALUES ('%s', '%s', '%s', '"MAC_FMT"', '%s', '%lld');"

#define SELECT_USER_FMT "SELECT *, "USER_TIME_FMT" FROM users " \
			"WHERE ("NOT_EXPIRED_FMT" AND email = '%s')"LIMIT_FMT";"

#define MSG_FMT "Password - %s, expiration time - %s"

struct str_sms_user {
	char username[USER_RAND_USERNAME_LEN + 1];
	char password[USER_RAND_PASSWORD_LEN + 1];
	char phone[USER_PHONESIZE + 1];
	uint32_t expiration;
	uint32_t user_time;
};

struct str_user {
	char username[USER_USERNAMENAMESIZE + 1];
	char email[USER_EMAILSIZE + 1];
	char password[USER_PASSWORDSIZE + 1];
	char phone[USER_PHONESIZE + 1];
	uint32_t expiration;
	uint32_t user_time;
};

int usr_get_user (sqlite3 *db, struct str_user *user, char *email);
int usr_add_user(struct redir_conn_t *conn);

int usr_get_sms_user(sqlite3 *db, struct str_sms_user *user, char *password);
int usr_add_sms_user(struct redir_conn_t *conn, char *phone, char *hexchall);
int usr_sms_user_exists(sqlite3 *db, char *phone, uint8_t *mac);

#endif //RUTX_USERS_H
