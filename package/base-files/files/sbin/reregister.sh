#!/bin/sh

. /lib/teltonika-functions.sh

reregister() {
	logger -t "$0" "Forcing LTE mode"

	ifdown ppp
	sleep 3

	#jeigu reikia isregistruojamas modemas
	local need_reregister=`uci get -q reregister.reregister.force_reregister`
	if [ "$need_reregister" == "1" ]; then
		logger -t "$0" "Reregister module"
		`/usr/sbin/gsmctl -A AT+COPS=2`
		sleep 1

		/etc/init.d/gsmd restart
		sleep 3

		for i in "1 2"; do
			sleep 5
			if [ "`check_type`" = "LTE" ]; then
				logger -t "$0" "Success"
				break
			fi
		done
	else

		for i in "1 2"; do
			sleep 5
			if [ "`check_type`" = "LTE" ]; then
				logger -t "$0" "Success"
				break
			fi
		done

		ifup ppp
	fi
}

check_type() {
	echo `gsmctl -t`
}

logger "$0" "Starting modem reregister manager"

INTERVAL=`uci get reregister.reregister.interval 2>/dev/null`
VIDPID=$(get_vidpid_tlt)

if [ "$VIDPID" != "$HUAWEI_LTE" -a "$VIDPID" != "$TELIT_LTE" -a "$VIDPID" != "$TELIT_LTE_V2" -a "$VIDPID" != "$QUECTEL" -a "$VIDPID" != "$QUECTEL_EC25" ]; then
	logger "$0" "Error. Not LTE modem: $VIDPID"
	exit 1
fi

enabled=`wan_section_enabled type mobile`

if [ "$enabled" == "0" ]; then
	logger -t "$0" "Error. LTE modem interface is not WAN"
	exit 1
fi

# Check GSMD
if [ "$(pidof gsmd)" ]
then
	echo "$0: gsmd is running"
else
	logger "$0" "Starting gsmd"
	/etc/init.d/gsmd start
	sleep 3
fi

while [ 1 ]
do
	sleep "$INTERVAL"
	if [ "`check_type`" != "LTE" ]; then
		reregister
	fi
done
