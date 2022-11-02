# Script to check operator selection state.
# It runs by operctl, when module already connected to operator
# In case SIM card suspend by client and we don't get reject message
# We have to restart modem by ourselfs

#!/bin/sh

timeout="30"
restart_timeout="450"
modem_id="$1"

pid=$(ps w | grep -v grep | grep -c "check_operator_selection_state.sh $modem_id")

if [ "$pid" != "2" ]; then
	exit 0
fi

while [ 1 ]; do

	state=$(gsmctl -n -A at+cops? -O "$modem_id" | cut -b 1-8)

	if [ "$state" == "+COPS: 2" ]; then
		logger "Deregistered from operator!"
		sleep $restart_timeout
		/sbin/mctl -r
		exit 0
	fi

	sleep $timeout

done
