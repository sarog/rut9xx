#!/bin/sh

#
# Display Cellular network type and signal strength with LEDs
#
# (c) 2014 Teltonika

. /lib/led_functions.sh

DEBUG=""
PID=$$
STATUS_LED_MAN_SCRIPT="/usr/sbin/statusledctrl"
NO_GSMD_LOG="" 						#"-n" to not show in log
sleep_delay=5
AUSTRALIAN=`uci -q get overview.show.australian`
AUSTRA_CONN=`gsmctl -t`
if [ "$AUSTRALIAN" = "1" ]; then
	if [ "$AUSTRA_CONN" = "LTE" ]; then
		level0=-120
		level1=-115
		level2=-105
		level3=-100
		level4=-90
	else
		level0=-109
		level1=-102
		level2=-93
		level3=-87
		level4=-78
	fi
else
	level0=-111
	level1=-97
	level2=-82
	level3=-67
	level4=-52
fi

UNSOLICITED_DATA="/tmp/unsolicited_data"
UNSOLICITED_LOCK="/var/run/unsolicited_data_lock"
UNSOLICITED_DEBUG="/tmp/unsolicited_log"

show_options()
{
	printf "usage: `basename $0`\n"
}

wait_for_signal()
{
	signal="$1"
	for i in 1 2 3 4 5 6 7 8 9 10; do
		[ "$signal" -eq "$signal" ] 2>/dev/null && break;
		sleep 3
		signal=$(gsmctl --signal)
	done
	echo "$signal"
}

# Enable Telit unsolicited messages. Command returns ERROR if requested too early
enable_unsolicited()
{
	. /lib/teltonika-functions.sh
	modem=$(get_vidpid_tlt)
	[ "$modem" == "$TELIT" -o "$modem" == "$TELIT_LTE" ] || return
	for i in 1 2 3 4 5 6 7 8 9 10; do
		[ $(gsmctl -A "AT+CMER=3,0,0,2,0") = "OK" ] && break;
		sleep 3
	done
}

display_signal()
{
	# do not show signal level if not registered
	local state=`gsmctl --netstate`
	if [ -z "$state" ] || [ "$state" != "registered (home)" ] &&
	   [ "$state" != "registered (roaming)" ]; then
		all_off
		return
	fi

	local signal="$1"

	# for some reason non-negative value might come here
	if [ `echo $signal | cut -c 1` != "-" ]; then
		signal="-$signal"
	fi

	if [ "$AUSTRALIAN" = "1" ]; then
		if [ "$AUSTRA_CONN" = "LTE" ]; then
			signal=`gsmctl -W`
		else
			signal=`gsmctl -X`
		fi
	fi
	# validate if number
	if echo "$signal" | grep -qE ^\-?[0-9]+$; then
		if [ "$signal" -gt $level0 ]
		then
			led_on $led0
		else
			led_off $led0
		fi
		if [ "$signal" -gt $level1 ]
		then
			led_on $led1
		else
			led_off $led1
		fi
		if [ "$signal" -gt $level2 ]
		then
			led_on $led2
		else
			led_off $led2
		fi
		if [ "$signal" -gt $level3 ]
		then
			led_on $led3
		else
			led_off $led3
		fi
		if [ "$signal" -gt $level4 ]
		then
			led_on $led4
		else
			led_off $led4
		fi
	else
		all_off
	fi

}

save_state() {
	name="$1"
	value="$2"
	file="$3"
	sed -i "/$name:=/d" "$file" 2>/dev/null
	echo "$name:=$value" >> "$file"
}

get_initial_data() {
	lock "/tmp/mutex"
	conntype=$(gsmctl --conntype)
	simstate=$(gsmctl --simstate)
	netstate=$(gsmctl --netstate)
	signal=$(gsmctl --signal)

	#echo "$$ s pries: "`cat $UNSOLICITED_LOCK` >> $UNSOLICITED_DEBUG
	lock $UNSOLICITED_LOCK
	#echo "$$ s locked: "`cat $UNSOLICITED_LOCK` >> $UNSOLICITED_DEBUG

	save_state "conntype" "$conntype" "$UNSOLICITED_DATA"
	save_state "simstate" "$simstate" "$UNSOLICITED_DATA"
	save_state "netstate" "$netstate" "$UNSOLICITED_DATA"

	lock -u $UNSOLICITED_LOCK 2>>$UNSOLICITED_DEBUG
	#echo "$$ s unlock $?: "`cat $UNSOLICITED_LOCK` >> $UNSOLICITED_DEBUG
	lock -u "/tmp/mutex"

	#Call itself to set initial led state
	/usr/sbin/ledsman.sh "simstate"
	/usr/sbin/ledsman.sh "signal" "$signal" "first"
}

get_value() {
	name="$1"
	file="$2"
	value=$(cat "$file" 2>/dev/null | grep "$name:=" | cut -d'=' -f2)
	[ "$DEBUG" = 1 ] && echo "LEDS get_value: '$name' '$value'" >> "$UNSOLICITED_DEBUG"
	echo "$value"
}

display_status() {
	local simstate
	local pinstate
	local connstate
	local netstate
	local conntype
	local nextmode=""

	#echo "$$ g pries: "`cat $UNSOLICITED_LOCK` >> $UNSOLICITED_DEBUG
	lock $UNSOLICITED_LOCK
	#echo "$$ g locked: "`cat $UNSOLICITED_LOCK` >> $UNSOLICITED_DEBUG

	simstate=$(get_value "simstate" "$UNSOLICITED_DATA")
	netstate=$(get_value "netstate" "$UNSOLICITED_DATA")
	conntype=$(get_value "conntype" "$UNSOLICITED_DATA")
	connstate=$(get_value "connstate" "$UNSOLICITED_DATA")

	lock -u $UNSOLICITED_LOCK 2>>$UNSOLICITED_DEBUG
	#echo "$$ g unlock $?: "`cat $UNSOLICITED_LOCK` >> $UNSOLICITED_DEBUG

	local ledmode="_R"
	if echo "$conntype" | grep -qE ^[-+/A-Z]+$; then
		if [ "$conntype" == "LTE" ]; then
			ledmode="_G"
		elif [ "$conntype" == "DC-HSPA+" ] || [ "$conntype" == "HSPA" ] || [ "$conntype" == "HSUPA" ] || [ "$conntype" == "HSDPA" ] \
			|| [ "$conntype" == "WCDMA" ] || [ "$conntype" == "UMTS" ] || [ "$conntype" == "HSPA+" ] || [ "$conntype" == "HSDPA/HSUPA" ] || [ "$conntype" == "HSDPA+HSUPA" ]; then
			ledmode="_Y"
		fi
	fi


	# for some reason modem responds with "connected" while without
	#  any sim or any connected service at all. For now, this seems to happen
	#  on EC25 devices only, so we do not allow to show connected mode when
	#  there is no service.
	# This could happen probably due new modem firmware..?
	if [ "$simstate" == "not inserted" ] || [ "$simstate" == "unknown" ]; then
		ledmode="B_GR"
		display_signal "inf"
	elif [ "$connstate" == "connected" ]; then
		ledmode="S$ledmode"
	elif [ "${netstate:0:10}" == "registered" ]; then
		ledmode="B$ledmode"
	else
		ledmode="B_GYR"
		display_signal "inf"
	fi

	[ "$DEBUG" = 1 ] && echo "LEDS display_status: '$ledmode'" >> "$UNSOLICITED_DEBUG"

	killall -9 $(basename "$STATUS_LED_MAN_SCRIPT") 2>/dev/null
	"$STATUS_LED_MAN_SCRIPT" "$ledmode" &
}

[ $(uci get -q system.@leds[0].enable) == 1 ] || return 1


name="$1"
value="$2"
param="$3"
[ "$DEBUG" = 1 ] && echo "LEDS input: '$name' '$value'" >> "$UNSOLICITED_DEBUG"
case "$1" in
	"simstate"|"conntype"|"connstate"|"netstate")
		display_status;;
	"signal")
		# Signal may not be available yet. Wait for it
		[ "$param" == "first" ] && value=$(wait_for_signal "$value")
		display_signal "$value"
		[ "$param" == "first" ] && enable_unsolicited
		;;
	"start")
		get_initial_data;;
	*)
		echo "$0: Unrecognized parameter '$1'";;
esac
