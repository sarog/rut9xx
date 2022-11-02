#include "chilli.h"
#ifdef ENABLE_GSM
#include "gsm.h"
#endif


/*!
 * Generate a n octet random string
 * @param dst The memory area to copy to
 * @param size The number of bytes to copy
 * @return 0 on success, -1 on fail.
 */

int usr_random_hex(char *dst, size_t size)
{
	size_t i = 0;
	int c;
	char x[3];
	FILE *file;

	if ((file = fopen("/dev/urandom", "r")) == NULL) {
		syslog(LOG_ERR, "%s: fopen(/dev/urandom)", strerror(errno));
		return -1;
	}

	for (i=0; i <= size-2; i++){
		c = fgetc(file);
		snprintf(x, 3, "%.2x", c);
		dst[i++] = x[0];
		dst[i] = x[1];
	}

	dst[size] = 0;

	fclose(file);
	return 0;
}

int usr_runscript(struct redir_conn_t *conn, char* script,
		char *username, char *password) {
	int status;
	uint32_t sessiontime;

	if ((status = chilli_fork(CHILLI_PROC_SCRIPT, script)) < 0) {
		syslog(LOG_ERR, "%s: forking %s", strerror(errno), script);
		return 0;
	}

	if (status > 0) { /* Parent */
		return 0;
	}

	set_env("DEV", VAL_STRING, tun(tun, 0).devname, 0);
	set_env("ADDR", VAL_IN_ADDR, &conn->ourip, 0);
	set_env("FRAMED_IP_ADDRESS", VAL_IN_ADDR, &conn->hisip, 0);
	set_env("SESSION_TIMEOUT", VAL_ULONG64, &conn->s_params.sessiontimeout, 0);
	set_env("IDLE_TIMEOUT", VAL_ULONG, &conn->s_params.idletimeout, 0);
	set_env("CALLING_STATION_ID", VAL_MAC_ADDR, conn->hismac, 0);
	set_env("CALLED_STATION_ID", VAL_MAC_ADDR, chilli_called_station(&conn->s_state), 0);
	set_env("WISPR_BANDWIDTH_MAX_UP", VAL_ULONG, &conn->s_params.bandwidthmaxup, 0);
	set_env("WISPR_BANDWIDTH_MAX_DOWN", VAL_ULONG, &conn->s_params.bandwidthmaxdown, 0);
	set_env("COOVACHILLI_MAX_INPUT_OCTETS", VAL_ULONG64, &conn->s_params.maxinputoctets, 0);
	set_env("COOVACHILLI_MAX_OUTPUT_OCTETS", VAL_ULONG64, &conn->s_params.maxoutputoctets, 0);
	set_env("COOVACHILLI_MAX_TOTAL_OCTETS", VAL_ULONG64, &conn->s_params.maxtotaloctets, 0);
	set_env("COOVACHILLI_WARNING_OCTETS", VAL_ULONG64, &conn->s_params.warningoctets, 0);

	sessiontime = mainclock_diffu(conn->s_state.start_time);
	set_env("SESSION_TIME", VAL_ULONG, &sessiontime, 0);
	sessiontime = mainclock_diffu(conn->s_state.last_up_time);
	set_env("IDLE_TIME", VAL_ULONG, &sessiontime, 0);
	set_env("USER_EXPIRATION_TIME", VAL_ULONG64, &conn->s_params.expiration, 0);
	set_env("PHONE", VAL_STRING, &conn->s_state.redir.phone, 0);
	set_env("EMAIL", VAL_STRING, &conn->s_state.redir.email, 0);

	if (username) {
		set_env("USER_NAME", VAL_STRING, username, 0);
	}
	if (_options.smsusers) {
		if (password)
			set_env("PASSWORD", VAL_STRING, password, 0);
#ifdef ENABLE_GSM
		if (_options.modemid)
			set_env("MODEM_ID", VAL_STRING, _options.modemid, 0);
#endif
	}

	if (execl(
#ifdef ENABLE_CHILLISCRIPT
			SBINDIR "/chilli_script", SBINDIR "/chilli_script", _options.binconfig,
#else
			script,
#endif
			script, (char *) 0) != 0) {
		syslog(LOG_ERR, "%s: exec %s failed", strerror(errno), script);
	}

	exit(0);
}

int usr_gen_message(char *msg, char *password)
{
	time_t now;

	if (_options.dynexpirationtime) {
		time(&now);
		now += _options.dynexpirationtime;
	}

	sprintf(msg, MSG_FMT, password, _options.dynexpirationtime ? ctime(&now) : "unlimited");

	return 0;
}

int _select_callback(void *str_user, int argc, char **argv, char **azColName)
{
	struct str_user *p_str_user = (struct str_user *)str_user;

	if (argc > 0) {
		strncpy(p_str_user->username, argv[COL_USER_NAME], USER_USERNAMENAMESIZE);
		p_str_user->username[USER_USERNAMENAMESIZE] = 0;
		strncpy(p_str_user->email, argv[COL_USER_EMAIL], USER_EMAILSIZE);
		p_str_user->email[USER_EMAILSIZE] = 0;
		strncpy(p_str_user->password, argv[COL_USER_PASS], USER_PASSWORDSIZE);
		p_str_user->password[USER_PASSWORDSIZE] = 0;
		strncpy(p_str_user->phone, argv[COL_USER_PHONE], USER_PHONESIZE);
		p_str_user->phone[USER_PHONESIZE] = 0;
		p_str_user->expiration = argv[COL_USER_EXPIRATION] ? atoll(argv[COL_USER_EXPIRATION]) : 0;
		p_str_user->user_time = argv[COL_USER_TIME] ? atoll(argv[COL_USER_TIME]) : 0;
	}

	return SQL_SUCCESS;
}

int usr_user_expired (sqlite3 *db, char *email, uint8_t *mac)
{
	int ret = USER_RET_SUCCESS;
	char *sql;
	sqlite3_stmt *stmt;

	asprintf(&sql, SELECT_NOT_EXPIRED_EMAIL_FMT, "users", email, MAC_ARG(mac));
	if (_options.debug)
		syslog(LOG_INFO, "[%s] SQL: %s", __FUNCTION__ , sql);

	ret = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
	if (ret != SQLITE_OK) {
		syslog(LOG_INFO, "[%s] SQL error: %s", __FUNCTION__ , sqlite3_errmsg(db));
		ret = USER_RET_ERROR;
		goto out;
	}

	if (sqlite3_step(stmt) == SQLITE_ROW)
		if (sqlite3_column_int(stmt, 0) > 0)
			ret = USER_RET_ALREADY;

	sqlite3_finalize(stmt);

out:
	free(sql);

	return ret;
}

int usr_get_user(sqlite3 *db, struct str_user *user, char *email)
{
	int ret;
	char *sql;
	char *err = 0;

	asprintf(&sql, SELECT_USER_FMT, email);
	if (_options.debug)
		syslog(LOG_INFO, "[%s] SQL: %s", __FUNCTION__ , sql);

	ret = sqlite3_exec(db, sql, _select_callback, user, &err);
	if (ret){
		syslog(LOG_INFO, "[%s] SQL error: %s\n", __FUNCTION__, err);
		sqlite3_free(err);
		free(sql);

		return SQL_FAIL;
	}

	free(sql);

	return SQL_SUCCESS;
}

int usr_add_user(struct redir_conn_t *conn)
{
	sqlite3 *db;
	char *sql = NULL;
	int ret	  = USER_RET_ERROR;
	char username[USER_USERNAMENAMESIZE + 1];

	if (!_options.usersdbpath || !(db = sqlopen(_options.usersdbpath))) {
		conn->response = REDIR_SIGNUP_FAILED;
		return USER_RET_ERROR;
	}

	if ((ret = usr_user_expired(db, conn->s_state.redir.email, conn->hismac)) != USER_RET_SUCCESS) {
		if (ret == USER_RET_ALREADY)
			conn->response = REDIR_SIGNUP_ALREADY;
		else
			conn->response = REDIR_SIGNUP_FAILED;

		goto out;
	}

	if (usr_random_hex(username, USER_RAND_USERNAME_LEN)) {
		ret = USER_RET_ERROR;
		goto out;
	}

#ifdef HAVE_OPENSSL
	char *hashed_password = hash_md5(conn->s_state.redir.signup_password);

	// If hashing fails, we can save unencrypted data as a fallback since
	// later we handle both encrypted and plaintext data.
	if (!hashed_password)
		hashed_password = strdup(conn->s_state.redir.signup_password);
#endif

	asprintf(&sql, INSERT_USERS_FMT, username, conn->s_state.redir.email,
#ifdef HAVE_OPENSSL
		 hashed_password,
#else
		 conn->s_state.redir.signup_password,
#endif
		 MAC_ARG(conn->hismac), conn->s_state.redir.phone,
		 _options.dynexpirationtime);

	if (_options.debug)
		syslog(LOG_INFO, "[%s] SQL: %s", __FUNCTION__, sql);

	if (!sqlexec(db, sql, NULL)) {
		conn->response = REDIR_SIGNUP_SUCCESS;
		ret	       = USER_RET_SUCCESS;
		if (_options.usersignup && !(conn->s_params.flags & NO_SCRIPT)) {
			if (_options.debug)
				syslog(LOG_DEBUG, "%s(%d): Calling user signup script: %s\n", __FUNCTION__, __LINE__,
				       _options.usersignup);

			usr_runscript(conn, _options.usersignup, conn->s_state.redir.email, NULL);
		}
	} else {
		ret = USER_RET_ERROR;
	}

#ifdef HAVE_OPENSSL
	free(hashed_password);
#endif
	free(sql);
out:
	sqlclose(db);

	return ret;
}

int _sms_select_callback(void *str_user, int argc, char **argv, char **azColName)
{
	struct str_sms_user *p_str_user = (struct str_sms_user *)str_user;

	if (argc > 0) {
		strncpy(p_str_user->username, argv[COL_SMS_USER_NAME], USER_RAND_USERNAME_LEN);
		p_str_user->username[USER_RAND_USERNAME_LEN] = 0;
		strncpy(p_str_user->password, argv[COL_SMS_USER_PASS], USER_RAND_PASSWORD_LEN);
		p_str_user->password[USER_RAND_PASSWORD_LEN] = 0;
		strncpy(p_str_user->phone, argv[COL_SMS_USER_PHONE], USER_PHONESIZE);
		p_str_user->phone[USER_PHONESIZE] = 0;
		p_str_user->expiration = argv[COL_SMS_USER_EXPIRATION] ? atoll(argv[COL_SMS_USER_EXPIRATION]) : 0;
		p_str_user->user_time = argv[COL_SMS_USER_TIME] ? atoll(argv[COL_SMS_USER_TIME]) : 0;
	}

	return SQL_SUCCESS;
}

int usr_get_sms_user (sqlite3 *db, struct str_sms_user *user, char * password)
{
	int ret;
	char *sql;
	char *err = 0;

	asprintf(&sql, SELECT_SMS_USER_FMT, password);
	if (_options.debug)
		syslog(LOG_INFO, "[%s] SQL: %s", __FUNCTION__ , sql);

	ret = sqlite3_exec(db, sql, _sms_select_callback, user, &err);
	if (ret){
		syslog(LOG_INFO, "[%s] SQL error: %s\n", __FUNCTION__, err);
		sqlite3_free(err);
		free(sql);

		return SQL_FAIL;
	}

	free(sql);

	return SQL_SUCCESS;
}

int usr_sms_user_exists (sqlite3 *db, char *phone, uint8_t *mac)
{
	int ret = USER_RET_ERROR;
	char *sql;
	sqlite3_stmt *stmt;

	if (phone != NULL && mac != NULL)
		asprintf(&sql, SELECT_NOT_EXPIRED_FMT, "sms_users", phone, MAC_ARG(mac));
	else if (mac != NULL)
		asprintf(&sql, SELECT_NOT_EXPIRED_MAC_FMT, "sms_users", MAC_ARG(mac));
	else
		goto out;

	if (_options.debug)
		syslog(LOG_INFO, "[%s] SQL: %s", __FUNCTION__ , sql);

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
		syslog(LOG_INFO, "[%s] SQL error: %s", __FUNCTION__ , sqlite3_errmsg(db));
		goto out;
	}

	if (sqlite3_step(stmt) == SQLITE_ROW)
		ret = sqlite3_column_int(stmt, 0) > 0 ? USER_RET_ALREADY : USER_RET_SUCCESS;

	sqlite3_finalize(stmt);

out:
	free(sql);

	return ret;
}

int usr_add_sms_user (struct redir_conn_t *conn, char *phone, char *hexchall)
{
	sqlite3 *db;
	char *sql = NULL;
	int ret = USER_RET_ERROR;
	char username[USER_RAND_USERNAME_LEN + 1], password[USER_RAND_PASSWORD_LEN + 1];

	if (!_options.usersdbpath || !(db = sqlopen(_options.usersdbpath)))
		return USER_RET_ERROR;

	if ((ret = usr_sms_user_exists(db, phone, conn->hismac)) != USER_RET_SUCCESS) {
		syslog(LOG_DEBUG, "user already exists: %d\n", ret);
		goto out;
	}

	if (usr_random_hex(username, USER_RAND_USERNAME_LEN) ||
			usr_random_hex(password, USER_RAND_PASSWORD_LEN))
	{
		ret = USER_RET_ERROR;
		goto out;
	}

	asprintf(&sql, INSERT_SMS_USERS_FMT, phone, password, MAC_ARG(conn->hismac),
			 phone, _options.dynexpirationtime);
	if (_options.debug)
		syslog(LOG_INFO, "[%s] SQL: %s", __FUNCTION__ , sql);

	if (!sqlexec(db, sql, NULL)) {
		ret = USER_RET_SUCCESS;
		if (_options.usersignup && !(conn->s_params.flags & NO_SCRIPT)) {
			if (_options.debug)
				syslog(LOG_DEBUG, "%s(%d): Calling user signup script: %s\n",
					   __FUNCTION__, __LINE__, _options.usersignup);

			usr_runscript(conn, _options.usersignup, username, password);
		}
#ifdef ENABLE_GSM
		char message[USER_MAX_MESSAGE_SIZE];

		usr_gen_message(message, password);
		if (gsm_send_sms(phone, message, _options.modemid ? _options.modemid :
				GSM_DEFAULT_USB_ID))
			ret = USER_RET_SMS_ERR;
#endif
	}
	else {
		ret = USER_RET_ERROR;
	}

	free(sql);
out:
	sqlclose(db);

	return ret;
}

int usr_get(char *username)
{
	return 0;
}
