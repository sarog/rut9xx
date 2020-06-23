#!/bin/sh
# Skriptas atliekantis wget nurodytu adresu ir jei wget nepraeina perkraunamas RUT9.

enable=`uci -q get wget_reboot.wget_reboot.enable`
timeout=`uci -q get wget_reboot.wget_reboot.timeout`
retries=`uci -q get wget_reboot.wget_reboot.retry`
host=`uci -q get wget_reboot.wget_reboot.host`
action=`uci -q get wget_reboot.wget_reboot.action`
time=`uci -q get wget_reboot.wget_reboot.time`



file="/tmp/wget_check_file"
sleep_time=$(( $time * 60))

DEBUG_LEVEL="0"

debug() {
	if [ "$DEBUG_LEVEL" == "1" ]; then
		logger -t wget_reboot.sh "$1"
	fi
}

if [ "$enable" == "1" ]; then
	debug "enabled"
	wget -q -T $timeout $host -O $file &> /dev/null

	if [ ! -s $file ];then
		debug "Can't wget URL."
		if [ $retries -gt 0 ]; then
			for i in `seq $(($retries -1))`
			do
				if [ "`uci -q get wget_reboot.wget_reboot.enable`" == "1" ]; then
					debug "wget retry - $i of $retries"
					wget -q -T $timeout $host -O $file &> /dev/null
					debug "go sleep $sleep_time"
					sleep $sleep_time
					if [ -s $file ]; then
						debug "Wget URL successful after $i retries."
						action="6"
						break
					fi
				else
					debug "Wget reboot not enabled"
					action="6"
					break
				fi
			done
	fi
		rm $file &> /dev/null

	case "$action" in
		"1")
			logger -t wget_reboot.sh "Rebooting router after $retries unsuccessful retries"
			reboot -v
			;;
		"2")
			logger -t wget_reboot.sh "Restarting modem after $retries unsuccessful retries"
			/etc/init.d/modem restart
			;;
		"3")
			logger -t wget_reboot.sh "Restarting mobile data connection after $retries unsuccessful retries"
			ifdown ppp
			sleep 1
			ifup ppp
			;;
		"4")
			logger -t wget_reboot.sh "Reregistering after $retries unsuccessful retries"
			ifdown ppp
			/usr/sbin/gsmctl -A AT+COPS=2
			/etc/init.d/gsmd restart
			;;
		"5")
			debug "Doing Nothing..."
			;;
		"6")
			logger -t wget_reboot.sh "Sending message after $retries unsuccessful retries"
			number=`uci -q get wget_reboot.wget_reboot.number`
			message=`uci -q get wget_reboot.wget_reboot.message`
			for onenumb in $number
			do				
				gsmctl -Ss "$onenumb $message"
			done
			;;
		"7")
			;;
		esac
	else
		debug "Wget URL successful."
		rm $file &> /dev/null
	fi
fi

