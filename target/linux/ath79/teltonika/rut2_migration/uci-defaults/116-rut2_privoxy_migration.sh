#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

USER_ACTION_FILE="/etc/privoxy/user_action_luci"
mkdir -p "$(dirname $USER_ACTION_FILE)"

UPLOADED_FILE="/lib/uci/upload/cbid.privoxy.privoxy.proxy_blocking_hosts"

fill_action_file() {
	local sec="$1"
	local enabled domain

	config_get enabled "$sec" enabled 1
	[ "$enabled" -ne 1 ] && return 0

	config_get domain "$sec" domen
	echo "$domain" >>"$USER_ACTION_FILE"
	uci_remove privoxy "$sec"
}

config_load privoxy
config_foreach fill_action_file rule

uci batch <<-EOF >/dev/null
	set privoxy.privoxy.confdir='/etc/privoxy'
	set privoxy.privoxy.logdir='/var/log'
	set privoxy.privoxy.filterfile='default.filter'
	set privoxy.privoxy.logfile='privoxy'
	set privoxy.privoxy.actionsfile='/var/run/user.action'
	set privoxy.privoxy.toggle='1'
	set privoxy.privoxy.enable_remote_toggle='1'
	set privoxy.privoxy.enable_remote_http_toggle='0'
	set privoxy.privoxy.enable_edit_actions='1'
	set privoxy.privoxy.enforce_blocks='0'
	set privoxy.privoxy.buffer_limit='4096'
	set privoxy.privoxy.forwarded_connect_retries='0'
	set privoxy.privoxy.accept_intercepted_requests='1'
	set privoxy.privoxy.allow_cgi_request_crunching='0'
	set privoxy.privoxy.split_large_forms='0'
	set privoxy.privoxy.keep_alive_timeout='300'
	set privoxy.privoxy.socket_timeout='300'
	set privoxy.privoxy.listen_address=':8118'
	set privoxy.privoxy.permit_access='lan'
	set privoxy.privoxy._user_action_file="$USER_ACTION_FILE"
EOF

uci_rename privoxy privoxy mode _mode
uci_commit privoxy

[ -f "$UPLOADED_FILE" ] && cat "$UPLOADED_FILE" >>"$USER_ACTION_FILE"

exit 0
