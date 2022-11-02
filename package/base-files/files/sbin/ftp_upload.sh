#!/bin/sh
# Upload file to FTP server

. /lib/functions.sh

APP_NAME="ftp_upload.sh"
CONFIG_GET="uci -q get ulogd.ftp"
LOG_FILE="/tmp/ftp_log"
LOGGER="logger -t $APP_NAME -s"
CLIENT=$(which ftpput)


config_load ulogd
config_get host ftp host
config_get sname ftp sname "/var/log/ulogd_wifi.log"
config_get dname ftp dname "traffic_log.tar.gz"
config_get username ftp username
config_get password ftp password
config_get port ftp port 21
config_get debug ftp debug
config_get delay ftp delay 6
config_get extra_name_info ftp extra_name_info
config_get custom_string ftp custom_string

archive_file(){
	tar -czf /tmp/$2 $1
	rotate_log $1
}

#Clean and reopen ulogd log file
rotate_log(){
	> $1
	#/bin/killall -HUP ulogd 2> /dev/null
}

[[ -z "$CLIENT" ]] && {
	$LOGGER "FTP client not found"
	exit 1
}

[[ -z "$host" ]] && {
	$LOGGER "No FTP host provided."
	exit 1
}

case "$extra_name_info" in
	mac)
		EXTRA=$(mnf_info --mac)
	;;
	serial)
		EXTRA=$(mnf_info --sn)
	;;
	custom)
		[[ -n "$custom_string" ]] && EXTRA="$custom_string"
	;;
esac

prefix=$(date +20%y%m%d_%H%M%S)
DEST_NAME="${prefix}${EXTRA:+_$EXTRA}_${dname}"
DEST_FILE="/tmp/$DEST_NAME"

archive_file "$sname" "$DEST_NAME"

COMMAND="ftpput ${username:+-u $username} ${password:+-p $password} ${port:+-P $port} \
			${debug:+ -v} ${host:+$host} ${DEST_NAME:+$DEST_NAME} ${DEST_FILE:+$DEST_FILE}"

for i in 1 2 3 4 5
do
	$COMMAND &> $LOG_FILE

	if [ "$?" -ne "0" ]; then
		cat "$LOG_FILE" | $LOGGER
		sleep $delay
	else
		$LOGGER "FTP upload successful."
		break
	fi
done

rm -rf "$DEST_FILE"