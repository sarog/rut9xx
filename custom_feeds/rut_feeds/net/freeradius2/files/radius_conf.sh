#!/bin/sh

. /lib/functions.sh

PREFIX="/usr"
EXEC_PREFIX="/usr"
SYSCONFDIR="/etc"
LOCALSTATEDIR="/var"
SBINDIR="/usr/sbin"
LOGDIR="/var/log"
RADDBDIR="/etc/freeradius2"
RADACCTDIR="/var/db/radacct"
NAME="radiusd"
CONFDIR="\${raddbdir}"
RUN_DIR="\${localstatedir}/run"
DB_DIR="\${raddbdir}"
LIBDIR="/usr/lib/freeradius2"
PIDFILE="\${run_dir}/\${name}.pid"
MAX_REQUEST_TIME=30
CLEANUP_DELAY=5
MAX_REQUESTS=1024
HOSTNAME_LOOKUPS="no"
ALLOW_CORE_DUMPS="no"
REGULAR_EXPRESSIONS="yes"
EXTENDED_EXPRESSIONS="yes"
CHECKRAD="\${sbindir}/checkrad"
PROXY_REUESTS="no"
INCLUDE1="\$INCLUDE clients.conf"
INCLUDE2="\$INCLUDE sites/"

RADIUS_CONF=$RADDBDIR/$NAME.conf


writeconfig1() {
	cat <<EOF
prefix=$PREFIX
exec_prefix=$EXEC_PREFIX
sysconfdir=$SYSCONFDIR
localstatedir=$LOCALSTATEDIR
sbindir=$SBINDIR
logdir=$LOGDIR
raddbdir=$RADDBDIR
radacctdir=$RADACCTDIR
name=$NAME
confdir=$CONFDIR
run_dir=$RUN_DIR
db_dir=$DB_DIR
libdir=$LIBDIR
pidfile=$PIDFILE
max_request_time=$MAX_REQUEST_TIME
cleanup_delay=$CLEANUP_DELAY
max_requests=$MAX_REQUESTS
hostname_lookups=$HOSTNAME_LOOKUPS
allow_core_dumps=$ALLOW_CORE_DUMPS
regular_expressions=$REGULAR_EXPRESSIONS
extended_expressions=$EXTENDED_EXPRESSIONS
checkrad=$CHECKRAD
proxy_requests=$PROXY_REUESTS
$INCLUDE1
$INCLUDE2

EOF
}

write_sections() {
	local AUTH_PORT ACC_PORT
	config_load "radius"
	config_get AUTH_PORT "general" "auth_port"
	config_get ACC_PORT "general" "acc_port"

	cat <<EOF
listen {
	type = auth
	ipaddr = *
	port = ${AUTH_PORT:-0}
}

listen {
	ipaddr = *
	port = ${ACC_PORT:-0}
	type = acct
}


log {
	destination = files
	file = \${logdir}/radius.log
	syslog_facility = daemon
	stripped_names = no
	auth = no
	auth_badpass = no
	auth_goodpass = no
}

security {
	max_attributes = 200
	reject_delay = 1
	status_server = yes
	allow_vulnerable_openssl = no
}

thread pool {
	start_servers = 5
	max_servers = 32
	min_spare_servers = 3
	max_spare_servers = 10
	max_requests_per_server = 0
}

modules {
	\$INCLUDE \${confdir}/modules/
	\$INCLUDE eap.conf
}

instantiate {

}

EOF
}

logger "config name $RADIUS_CONF"
writeconfig1 > $RADIUS_CONF
write_sections >> $RADIUS_CONF









