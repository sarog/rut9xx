#!/bin/sh

. /lib/teltonika-functions.sh
. /lib/netifd/ndis-general.sh

init() {
	if [ -f /var/run/ndisconn.pid ]
	then
		logger -t "$0" "already running, exiting"
		exit 0
	fi
	echo "$$" > /var/run/ndisconn.pid
}

disconnected() {
	echo "Disconnected"
	CONNECTED=0
}

connected() {
	echo "Connected"
	CONNECTED=1
}

get_auth() {
	if [ "$AUTH" = "pap" ]; then
		auth=1
	elif [ "$AUTH" = "chap" ]; then
		auth=2
	fi
}

check_connection() {
	#Check if data session is active
	local conn=`$CON_COMMAND | grep "IPV4" | awk -F" " '{print $2}' | awk -F"," '{print $1}'`
	local operator=`/usr/sbin/gsmctl -gn`
	local oper_res="$?"

	if [ -z "$conn" ] || [ -z "$operator" ]; then
		#Retry if no answer was received
		sleep 1
		conn=`$CON_COMMAND | grep "IPV4" | awk -F" " '{print $2}' | awk -F"," '{print $1}'`
		sleep 1
		operator=`/usr/sbin/gsmctl -gn`
		oper_res="$?"
	fi

	if [ "$operator" == "registered (home)" ] || [ "$operator" == "registered (roaming)" ]; then
		if [ "$conn" != "1" ]; then
			logger -t "$0" "NDIS connection inactive ($conn). Starting connection"
			local ipv6disable=`uci -q get network.ppp.pdptype`
			
			if [ "$ipv6disable" == "1" ]; then
			
				local cmd="AT+CGDCONT=1,\"IP\""
				gsmctl -A "$cmd" > /dev/null
				ret=$?
				if [ "$ret" != "0" ]; then
					logger -t "$0" "Error sending NDIS cgdcont=1, IP command. Bypassing gsmd"
					echo -ne "$cmd\r" | microcom -s 115200 "$DEVICE" -t 100
				fi
			
				local cmd="AT+CGDCONT=16,\"IP\""
				gsmctl -A "$cmd" > /dev/null
				ret=$?
				if [ "$ret" != "0" ]; then
					logger -t "$0" "Error sending NDIS cgdcont=16, IP command. Bypassing gsmd"
					echo -ne "$cmd\r" | microcom -s 115200 "$DEVICE" -t 100
				fi
			
				local cmd="AT+CGDCONT=2"
				gsmctl -A "$cmd" > /dev/null
				ret=$?
				if [ "$ret" != "0" ]; then
					logger -t "$0" "Error sending NDIS cgdcont=2 command. Bypassing gsmd"
					echo -ne "$cmd\r" | microcom -s 115200 "$DEVICE" -t 100
				fi
			
				sleep 3
			else
				local cmd="AT+CGDCONT=1,\"IPV4V6\""
				gsmctl -A "$cmd" > /dev/null
				ret=$?
				if [ "$ret" != "0" ]; then
					logger -t "$0" "Error sending NDIS cgdcont=1,IPV4V6 command. Bypassing gsmd"
					echo -ne "$cmd\r" | microcom -s 115200 "$DEVICE" -t 100
				fi
				
				sleep 1
			fi

			local cmd="AT^NDISDUP=1,1,\"$APN\",\"$USER\",\"$PSW\",$auth"
			gsmctl -A "$cmd" > /dev/null
			ret=$?
			if [ "$ret" != "0" ]; then
				logger -t "$0" "Error sending NDIS connect command. Bypassing gsmd"
				echo -ne "$cmd\r" | microcom -s 115200 "$DEVICE" -t 100
			fi

			sleep 6
			#Start DHCP client
			pid=`ps | grep "udhcpc" | grep "eth2.pid" | awk -F ' ' '{print $1}'`
			kill -USR1 $pid
		else
			CONNECTED=1
		fi
		DOWN=0
	else
		if [ "$oper_res" == 0 ] && [ $DOWN -ne 1 ]; then
			DOWN=1
			pid=`ps | grep "udhcpc" | grep "eth2.pid" | awk -F ' ' '{print $1}'`
			kill -USR2 $pid
			gsmctl -A "AT^NDISDUP=1,0" > /dev/null
			logger -t "$0" "Operator not available ($operator)."
		fi
	fi
}

init

logger "$0" "Starting Huawei NDIS connection manager"

COMMAND="gsmctl -n -A AT+CREG?"
CON_COMMAND="gsmctl -n -A AT^NDISSTATQRY?"
OLD_COMMAND="echo -ne \"AT+CREG?\r\" | microcom -s 115200 "$DEVICE" -t 1000"
OLD_CON_COMMAND="echo -ne \"AT^NDISSTATQRY?\r\" | microcom -s 115200 "$DEVICE" -t 1000"
INTERVAL=10
DEVICE=`uci -q get network.ppp.device`
APN=`uci -q get network.ppp.apn`
AUTH=`uci -q get network.ppp.auth_mode`
auth=0
USER=`uci -q get network.ppp.username`
PSW=`uci -q get network.ppp.password`
SERVICE=`uci -q get network.ppp.service`
IFNAME=`uci -q get network.ppp.ifname`
VIDPID=$(get_ext_vidpid_tlt)
CONNECTED=0
DOWN=0

# Check GSMD
if [ "$(pidof gsmd)" ]
then
	echo "$0: gsmd is running"
else
	logger "$0" "Starting gsmd"
	/etc/init.d/gsmd start
	sleep 3
fi

get_auth

trap disconnected SIGUSR1
trap connected SIGUSR2

#Wait in case there was disconnection
sleep 2

while [ 1 ]
do
	check_connection
	#sleep "$INTERVAL"
	sleep 2
	tick=$INTERVAL
	while [ "$tick" -gt 1 ] && [ "$CONNECTED" = 1 ]; do
		sleep 2
		tick=`expr $tick - 1`
	done

done
