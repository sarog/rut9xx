#!/bin/sh

RETRY=5
COUNT=0
DEVICES=0

count_devices() {
	DEVICES=`ls /dev/ | grep -c ttyACM`
}


while [ 1 ]; do
	count_devices
	if [ $DEVICES -lt 4 ]; then
		logger -t "modem_check" "Modem not start correctly"
		COUNT=$((COUNT+1))
	else
		imei=`gsmctl -n -i`
		if [ "$imei" == "" -o "$imei" == "Timeout" ]; then
			logger -t "modem_check" "Looks like modem not responce"
			COUNT=$((COUNT+1))
		else
			COUNT=0
		fi
	fi

	if [ $COUNT -gt $RETRY ]; then
		logger -t "modem_check" "Restarting modem"
		/etc/init.d/modem restart &
		exit
	fi

	sleep 30
done
