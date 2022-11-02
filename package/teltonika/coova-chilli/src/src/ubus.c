
#include "chilli.h"
#include "options.h"


static int chilli_list_method(struct ubus_context *ctx, struct ubus_object *obj,
							  struct ubus_request_data *req, const char *method, struct blob_attr *msg);
static int chilli_logout_method(struct ubus_context *ctx, struct ubus_object *obj,
								struct ubus_request_data *req, const char *method, struct blob_attr *msg);

enum {
	CHILLI_IP,
	CHILLI_MAC,
	CHILLI_SESSION_ID,
	CHILLI_MAX
};

static const struct blobmsg_policy chilli_default_policy[] = {
		[CHILLI_IP]  = {.name = "ip", .type = BLOBMSG_TYPE_STRING},
		[CHILLI_MAC] = {.name = "mac", .type = BLOBMSG_TYPE_STRING},
		[CHILLI_SESSION_ID]   = {.name = "sessionid", .type = BLOBMSG_TYPE_STRING}
};

static const struct ubus_method chilli_methods[] = {
		UBUS_METHOD("list", chilli_list_method, chilli_default_policy),
		UBUS_METHOD("logout", chilli_logout_method, chilli_default_policy),
};

static struct ubus_object_type chilli_object_type =
		UBUS_OBJECT_TYPE("chilli_obj_type", chilli_methods);

struct ubus_object chilli_object = {
		.name      = "chilli",
		.type      = &chilli_object_type,
		.methods   = chilli_methods,
		.n_methods = ARRAY_SIZE(chilli_methods),
};

static int parse_mac(uint8_t *mac, char *string) {
	unsigned int temp[PKT_ETH_ALEN];
	char macstr[RADIUS_ATTR_VLEN];
	int macstrlen;
	int i;

	if ((macstrlen = strlen(string)) >= (RADIUS_ATTR_VLEN-1)) {
		fprintf(stderr, "%s: bad MAC address\n", string);
		return -1;
	}

	memcpy(macstr, string, macstrlen);
	macstr[macstrlen] = 0;

	for (i=0; i<macstrlen; i++)
		if (!isxdigit((int) macstr[i]))
			macstr[i] = 0x20;

	if (sscanf(macstr, "%2x %2x %2x %2x %2x %2x",
			   &temp[0], &temp[1], &temp[2],
			   &temp[3], &temp[4], &temp[5]) != 6) {
		fprintf(stderr, "%s: bad MAC address\n", string);
		return -1;
	}

	for (i = 0; i < PKT_ETH_ALEN; i++)
		mac[i] = temp[i];

	return 0;
}

static void chilli_session_params(struct session_state *state, struct session_params *params,
								  struct blob_buf *b)
{
	time_t starttime = state->start_time;

	blobmsg_add_string(b, "sessionId", state->sessionid);
	blobmsg_add_string(b, "userName", state->redir.username);
	blobmsg_add_u64(b, "startTime", (uint64_t) mainclock_towall(starttime));
	blobmsg_add_u64(b, "sessionTimeout", params->sessiontimeout);
	blobmsg_add_u64(b, "terminateTime", params->sessionterminatetime);
	blobmsg_add_u32(b, "idleTimeout", params->idletimeout);
#ifdef ENABLE_IEEE8021Q
	if (_options.ieee8021q && state->tag8021q) {
		blobmsg_add_u16(b, "vlan", ntohs(state->tag8021q & PKT_8021Q_MASK_VID));
	}
#endif
	if (params->maxinputoctets) {
		blobmsg_add_u64(b, "maxInputOctets", params->maxinputoctets);
	}
	if (params->maxoutputoctets) {
		blobmsg_add_u64(b, "maxOutputOctets", params->maxoutputoctets);
	}
	if (params->maxtotaloctets) {
		blobmsg_add_u64(b, "maxTotalOctets", params->maxtotaloctets);
	}
	if (params->bandwidthmaxdown) {
		blobmsg_add_u64(b, "maxDwBandwidth", params->bandwidthmaxdown);
	}
	if (params->bandwidthmaxup) {
		blobmsg_add_u64(b, "maxUpBandwidth", params->bandwidthmaxup);
	}
}

static void chilli_session_acct(struct session_state *state, struct blob_buf *b)
{
	uint32_t inoctets = state->input_octets;
	uint32_t outoctets = state->output_octets;
	uint32_t ingigawords = (state->input_octets >> 32);
	uint32_t outgigawords = (state->output_octets >> 32);
	uint32_t sessiontime;
	uint32_t idletime;

	sessiontime = mainclock_diffu(state->start_time);
	idletime    = mainclock_diffu(state->last_up_time);

	blobmsg_add_u32(b, "sessionTime", !state->authenticated ? 0 : sessiontime);
	blobmsg_add_u32(b, "idleTime", !state->authenticated ? 0 : idletime);
	blobmsg_add_u32(b, "inputOctets", !state->authenticated ? 0 : inoctets);
	blobmsg_add_u32(b, "outputOctets", !state->authenticated ? 0 : outoctets);
	blobmsg_add_u32(b, "inputGigawords", !state->authenticated ? 0 : ingigawords);
	blobmsg_add_u32(b, "outputGigawords", !state->authenticated ? 0 : (long)outgigawords);
	blobmsg_add_string(b, "viewPoint", _options.swapoctets ? "nas" : "client");
}

static void chilli_getinfo(struct app_conn_t *appconn, struct blob_buf *b) {
	if (appconn->s_state.authenticated) {
		void *i = blobmsg_open_table(b, "session");
		chilli_session_params(&appconn->s_state, &appconn->s_params, b);
		blobmsg_close_table(b, i);

		i = blobmsg_open_table(b, "accounting");
		chilli_session_acct(&appconn->s_state, b);
		blobmsg_close_table(b, i);
	}
}

void chilli_form_blob(struct blob_buf *b, struct app_conn_t *appconn, struct dhcp_conn_t *conn) {
	char tmp_buff[64];

	if (!appconn && conn)
		appconn = (struct app_conn_t *)conn->peer;

	if ((!appconn || !appconn->inuse)) {
		return;
	} else if (conn && !conn->inuse) {
		return;
	} else {
		void *i = blobmsg_open_table(b, NULL);
		if (appconn) {
			blobmsg_add_u32(b, "nasPort", appconn->unit);
			blobmsg_add_u8(b, "clientState", appconn->s_state.authenticated);
			blobmsg_add_string(b, "ipAddress", inet_ntoa(appconn->hisip));
			if (appconn->s_state.redir.userurl[0]) {
				blobmsg_add_string(b, "url", appconn->s_state.redir.userurl);
			}
		}

		if (conn) {
			sprintf(tmp_buff, MAC_FMT, MAC_ARG(conn->hismac));
			blobmsg_add_string(b, "macAddress", tmp_buff);
			blobmsg_add_string(b, "dhcpState", state2name(conn->authstate));
		}

		if (appconn) {
			chilli_getinfo(appconn, b);
		}

		blobmsg_close_table(b, i);
	}
}

void unused (void){
	return;
}

static int chilli_list_method(struct ubus_context *ctx, struct ubus_object *obj,
							  struct ubus_request_data *req, const char *method,
							  struct blob_attr *msg)
{
	struct cmdsock_request req_params = { 0 };
	struct app_conn_t *appconn = NULL;
	struct dhcp_conn_t *dhcpconn = NULL;
	struct blob_attr *tb[CHILLI_MAX];
	struct blob_buf b = { 0 };
	int crt = 0;
	void *ses;

	blob_buf_init(&b, 0);
	blobmsg_parse(chilli_default_policy, ARRAY_SIZE(chilli_default_policy),
				  tb, blob_data(msg), blob_len(msg));

	if (tb[CHILLI_IP]) {
		crt = 1;
		if (!inet_pton(AF_INET, blobmsg_data(tb[CHILLI_IP]), &req_params.ip)) {
			return UBUS_STATUS_INVALID_ARGUMENT;
		}
	}

	if (tb[CHILLI_MAC]) {
		if (parse_mac(req_params.mac, blobmsg_data(tb[CHILLI_MAC]))) {
			return UBUS_STATUS_INVALID_ARGUMENT;
		}

		crt = 1;
	}

	if (tb[CHILLI_SESSION_ID]) {
		crt = 1;
		strlcpy(req_params.d.sess.sessionid, blobmsg_data(tb[CHILLI_SESSION_ID]),
				sizeof(req_params.d.sess.sessionid));
	}

	ses = blobmsg_open_array(&b, "sessions");
	appconn = find_app_conn(&req_params, &crt);
	if (appconn) {
		dhcpconn = (struct dhcp_conn_t *)appconn->dnlink;
		chilli_form_blob(&b, appconn, dhcpconn);
	} else if (!crt) {
		if (dhcp) {
			dhcpconn = dhcp->firstusedconn;
			while (dhcpconn) {
				chilli_form_blob(&b, NULL, dhcpconn);
				dhcpconn = dhcpconn->next;
			}
		}
	}

	blobmsg_close_array(&b, ses);
	ubus_send_reply(ctx, req, b.head);
	blob_buf_free(&b);

	return UBUS_STATUS_OK;
}

static int chilli_logout_method(struct ubus_context *ctx, struct ubus_object *obj,
								struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
	struct cmdsock_request req_params;
	struct app_conn_t *appconn = NULL;
	struct blob_attr *tb[CHILLI_MAX];
	int count = 0;

	memset(&req_params, 0, sizeof(req_params));
	blobmsg_parse(chilli_default_policy, ARRAY_SIZE(chilli_default_policy),
				  tb, blob_data(msg), blob_len(msg));

	if (tb[CHILLI_IP]) {
		count++;
		if (!inet_pton(AF_INET, blobmsg_data(tb[CHILLI_IP]), &req_params.ip)) {
			return UBUS_STATUS_INVALID_ARGUMENT;
		}
	}

	if (tb[CHILLI_MAC]) {
		if (parse_mac(req_params.mac, blobmsg_data(tb[CHILLI_MAC]))) {
			return UBUS_STATUS_INVALID_ARGUMENT;
		}

		count++;
	}

	if (tb[CHILLI_SESSION_ID]) {
		count++;
		strlcpy(req_params.d.sess.sessionid, blobmsg_data(tb[CHILLI_SESSION_ID]),
				sizeof(req_params.d.sess.sessionid));
	}

	if (count == 0) {
		return UBUS_STATUS_INVALID_ARGUMENT;
	}

	appconn = find_app_conn(&req_params, 0);
	if (_options.debug) {
		syslog(LOG_DEBUG, "%s(%d): looking to logout session %s", __FUNCTION__, __LINE__,
			   inet_ntoa(req_params.ip));
	}

	if (appconn) {
		if (_options.debug) {
			syslog(LOG_DEBUG, "%s(%d): found %s %s", __FUNCTION__, __LINE__,
				   inet_ntoa(appconn->hisip), appconn->s_state.sessionid);
		}

		terminate_appconn(appconn, RADIUS_TERMINATE_CAUSE_ADMIN_RESET);
	}

	return UBUS_STATUS_OK;
}

void ubus_disconnect_cb(struct ubus_context *ubus)
{
	int ret;

	if ((ret = ubus_reconnect(ubus, NULL)))
	{
		syslog(LOG_ERR, "Cannot reconnect to UBus: %s", ubus_strerror(ret));
		ubus_free(ubus);
	}
}

void chilli_ubus_add_obj(struct ubus_context *ctx)
{
	int ret;

	if ((ret = ubus_add_object(ctx, &chilli_object))) {
		syslog(LOG_ERR, "Failed to add object: %s", ubus_strerror(ret));
	}

	ctx->connection_lost = ubus_disconnect_cb;
}


// main hostapd event handler.
// the general approach of using hostapd might be inefficient, as hostapd
// is kind of a fire hose when it comes to the events it sends out.
static int hostapd_event_handle(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{

	// unused vars
	UNUSED(ctx);
	UNUSED(obj);
	UNUSED(req);

	// coova-chilli functionality related vars
	struct cmdsock_request req_params;
	struct app_conn_t *appconn = NULL;

	// blobmsg parsing related vars
	struct blob_attr *cur;
	int rem;

	// check to see if we're receiving a dissasoc call
	// from hostapd
	if (strcmp(method, CHILLI_UBUS_HAP_DISASSOC) != 0) {
		return UBUS_STATUS_OK;
	}

	// at this point, we know that we are dealing with data that is
	// related to a disconnect event

	// this loop is only really concerned with
	// the first blobvalue of *msg.
	// which may or may not be our MAC addr
	blobmsg_for_each_attr(cur, msg, rem) {

		// check if we have field named 'address' (contains our mac addr).
		// if not, break.
		if (strcmp(blobmsg_name(cur), CHILLI_UBUS_HAP_ADDR)) {
			continue;
		}

		// use the same methods as above to remove the disconnect from coova-chilli
		// internally.
		if (parse_mac(req_params.mac, blobmsg_get_string(cur))) {
			syslog(LOG_ERR, "Unable to parse incoming mac address from hostapd");
			break;
		}

		appconn = find_app_conn(&req_params, 0);

		if (_options.debug) {
			syslog(LOG_DEBUG, "%s(%d): looking to logout session %s", __FUNCTION__, __LINE__,
				inet_ntoa(req_params.ip));
		}

		if (!appconn) {
			break;
		}

		// at this point we have a valid appconn obj

		if (_options.debug) {
			syslog(LOG_DEBUG, "%s(%d): found %s %s", __FUNCTION__, __LINE__,
				   inet_ntoa(appconn->hisip), appconn->s_state.sessionid);
		}

		terminate_appconn(appconn, RADIUS_TERMINATE_CAUSE_LOST_SERVICE);

		// we can break here since we
		// got everything we needed out of *msg.
		break;
	}

	return UBUS_STATUS_OK;
}

// tries to subscribe a single *ubus_subscriber and an iface pair to hostapd
static int chilli_ubus_singlesub_hostapd(struct ubus_context *ctx, struct ubus_subscriber *event, char *iface)
{
	// null-check
	if (iface == NULL || iface[0] == '\0') {
		syslog(LOG_DEBUG, "%s given null iface argument.", __FUNCTION__);
		return 1;
	}

	uint32_t u_obj_id;

	char obj_name_buf[CHILLI_UBUS_OBJ_BUFSIZ];

	// set event handler for ubus_subscriber
	event->cb = hostapd_event_handle;

	// prepare ubus obj string. i,e take the iface we got and add the hostapd base ubus
	// obj name and a dot before it
	snprintf(obj_name_buf, CHILLI_UBUS_OBJ_BUFSIZ, "%s.%s", CHILLI_UBUS_HAP_OBJ, iface);

	// try to look for ubus hostapd obj
	if (ubus_lookup_id(ctx, obj_name_buf, &u_obj_id)) {
		syslog(LOG_ERR, "Failed to find ubus object: %s", obj_name_buf);
		return 1;
	}

	// try to register ubus subscriber
	if (ubus_register_subscriber(ctx, event)) {
		syslog(LOG_ERR, "Failed to register ubus subscriber for %s", obj_name_buf);
		return 1;
	}

	// try to subscribe
	if (ubus_subscribe(ctx, event, u_obj_id) != UBUS_STATUS_OK) {
		syslog(LOG_ERR, "Failed to subscribe to %s", obj_name_buf);
		return 1;
	}

	syslog(LOG_INFO, "Successfully subscribed to %s", obj_name_buf);
	return 0;

}


// function that subscribes to any relevant hostapd ubus objects. This is used to track 
// Wifi connects/disconnects in order to be able to remove user from session list
// if they disconnect from the wifi itself.
void chilli_ubus_subscribe_hostapd(struct ubus_context *ctx, struct options_t options)
{
	// NOTE: i know there's the _options global, accessible from here, but having the options be passed
	// as an argument is better practice, especially for maintainability


	// null-check. Something has probably gone very wrong if this gets triggered.
	if (options.dhcpif == NULL || options.dhcpif[0] == '\0') {
		syslog(LOG_ERR, "Unable to add hostapd listeners. DHCP iface not set.");
		return;
	}

	static struct ubus_subscriber hostapd_event; // ubus subscription object

	// try to subscribe to the hostapd instance of the main iface.
	// if this fails, move on.
	chilli_ubus_singlesub_hostapd(ctx, &hostapd_event, options.dhcpif);

	// basically do the same as above, but with the idea that all data of note is stored as arrays
	#ifdef ENABLE_MULTILAN
		static struct ubus_subscriber hostapd_multi_events[MAX_MOREIF]; // array of additional ubus sub objs

		// for every additional interface that we got
		for (int i = 0; i < MAX_MOREIF; i++) {
			// try to subscribe to additinoal interface, move on if failed.
			chilli_ubus_singlesub_hostapd(ctx, &hostapd_multi_events[i], options.moreif[i].dhcpif);
		}
	#endif
}
