#!/bin/sh
# (C) 2014 Teltonika

. /lib/functions.sh
. /usr/share/libubox/jshn.sh
. /lib/teltonika-gpio-functions.sh
ACTION=$1
NAME=$2
gpio=""

# logger -s "action == $1 || name == $2"

if [ -e "/sys/bus/i2c/devices/0-0074/gpio" ]; then
	GPIO_LIST="SIM	DOUT1	DOUT2	DIN1	DIN2	MON	MRST	SDCS	RS485_R"
	GPIO_PINS="55	56	57	59	58	60	61	62	63"
else
	GPIO_LIST="SIM	DOUT1	DOUT2	DIN1	DIN2	MON	MRST	DOUT3	DIN3	RS485_R	SDCS	HWRST"
	GPIO_PINS="30	31	32	21	19	33	34	35	2	36	37	38"
fi

usage () {
	echo "GPIO control aplication"
	echo -e "\tUsage: $0 <ACTION> <NAME>"
	echo -e "\tACTION - set, clear, get, export, invert, dirout, dirin, getpin"
	echo -e "\tNAME - $GPIO_LIST"
}

validate() {
	id=1
	for i in $GPIO_LIST; do
		if [ "$1" = "$i" ]; then
			pin=`echo $GPIO_PINS | awk -F " " -v x=$id '{print $x}'`
			gpio=$pin
			return
		fi
		id=`expr $id + 1`
	done
	echo "$0: GPIO $1 not supported"
	exit 1
}

do_led() {
	local name
	local sysfs
	config_get name $1 name
	config_get sysfs $1 sysfs
	[ "$name" == "$NAME" -o "$sysfs" = "$NAME" -a -e "/sys/class/leds/${sysfs}" ] && {
		[ "$ACTION" == "set" ] &&
			echo 1 >/sys/class/leds/${sysfs}/brightness \
			|| echo 0 >/sys/class/leds/${sysfs}/brightness
		exit 0
	}
}

func_set() {
	if [ "$NAME" == "DOUT1" ] || [ "$NAME" == "DOUT2" ] || [ "$NAME" == "DOUT3" ]; then
		output_active_state=`uci get ioman.@ioman[0].active_"$NAME"_status`
		if [ "$output_active_state" == "1" ]; then
			gpio_write_tlt $1 1
		else
			gpio_write_tlt $1 0
		fi
	else
		gpio_write_tlt $1 1
	fi
}

func_clear() {
	if [ "$NAME" == "DOUT1" ] || [ "$NAME" == "DOUT2" ] || [ "$NAME" == "DOUT3" ]; then
		output_active_state=`uci get ioman.@ioman[0].active_"$NAME"_status`
		if [ "$output_active_state" == "1" ]; then
			gpio_write_tlt $1 0
		else
			gpio_write_tlt $1 1
		fi
	else
		gpio_write_tlt $1 0
	fi
}

func_get() {
	value=`gpio_read_tlt $1`
	if [ "$value" == "-1" ]; then
		echo "1"
	else
		output_active_state=`uci get ioman.@ioman[0].active_"$NAME"_status`
		if [ "$output_active_state" == "0" ]; then
			value=$((!$value))
		fi
		echo $value
	fi
}

func_export() {
	gpio_export_tlt $1
}

func_invert() {
	gpio_invert_tlt $1
}

func_dirin() {
	gpio_setdir_tlt $1 in
}

func_dirout() {
	gpio_setdir_tlt $1 out
}

func_getpin() {
	echo $1
}

if [ "$#" != 2 ] || [ "$ACTION" != "set" -a "$ACTION" != "clear" -a "$ACTION" != "get" \
	 -a "$ACTION" != "export" -a "$ACTION" != "invert" -a "$ACTION" != "dirin" -a "$ACTION" != "dirout" -a "$ACTION" != "getpin" ]; then
	usage
	exit 1
fi

validate $NAME
if [ "$NAME" == "SIM" ]; then
	json_init
	json_load "$(ubus call sim get)"
	json_get_var sim sim

	if [ "$ACTION" == "get" ]; then
		if [ "$sim" == "2" ]; then
			echo "0"
		else
			echo "1"
		fi
	elif [ "$ACTION" == "set" ] && [ "$sim" == "2" ]; then
		ubus call sim change
	elif [ "$ACTION" == "clear" ] && [ "$sim" == "1" ]; then
		ubus call sim change
	fi
else
	func_$ACTION $gpio
fi

if [ "$ACTION" == "invert" ]; then
	value=`gpio_read_tlt "$gpio"`
	if [ "$value" == "0" ]; then
		value=off
		oldvalue=on
	else
		value=on
		oldvalue=off
	fi
fi

if [ "$NAME" == "DOUT1" ]; then
	if [ "$ACTION" == "set" ]; then
			/usr/bin/eventslog -i -t EVENTS -n "Output" -e "Digital OC output on"
	elif [ "$ACTION" == "clear" ]; then
			/usr/bin/eventslog -i -t EVENTS -n "Output" -e "Digital OC output off"
	elif [ "$ACTION" == "invert" ]; then
		/usr/bin/eventslog -i -t EVENTS -n "Output" -e "Digital OC output was inverted: from $oldvalue to $value"
	fi
elif [ "$NAME" == "DOUT2" ]; then
	if [ "$ACTION" == "set" ]; then
			/usr/bin/eventslog -i -t EVENTS -n "Output" -e "Digital relay output on"
	elif [ "$ACTION" == "clear" ]; then
			/usr/bin/eventslog -i -t EVENTS -n "Output" -e "Digital relay output off"
	elif [ "$ACTION" == "invert" ]; then
		/usr/bin/eventslog -i -t EVENTS -n "Output" -e "Digital relay output was inverted: from $oldvalue to $value"
	fi
elif [ "$NAME" == "DOUT3" ]; then
	if [ "$ACTION" == "set" ]; then
			/usr/bin/eventslog -i -t EVENTS -n "Output" -e "Digital 4PIN output on"
	elif [ "$ACTION" == "clear" ]; then
			/usr/bin/eventslog -i -t EVENTS -n "Output" -e "Digital 4PIN output off"
	elif [ "$ACTION" == "invert" ]; then
		/usr/bin/eventslog -i -t EVENTS -n "Output" -e "Digital 4PIN output was inverted: from $oldvalue to $value"
	fi
fi
