#!/bin/sh

# This script contains general Teltonika-specific
# bash functions to be included in other scripts
# in order to avoid multiple funtion definitions.

# Exports specified GPIO
#
# @param $1 - GPIO number
#
# @return - 0 on success; 1 on error;
gpio_export_tlt() {	
	local gpio=$1
	local ret=""
	ret=`echo $gpio 2>/dev/null > /sys/class/gpio/export`
	return $?
}

# Unexports specified GPIO
#
# @param $1 - GPIO number
#
# @return - 0 on success; 1 on error;
gpio_unexport_tlt() {
	local gpio=$1
	local ret=""
	ret=`echo $gpio 2>/dev/null > /sys/class/gpio/unexport`
	return $?
}

# Reads specified GPIO
#
# @param $1 - GPIO number
#
# @return - GPIO current value on success or
# -1 on error
gpio_read_tlt() {
	local gpio=$1
	local value=""
	value=`cat 2>/dev/null /sys/class/gpio/gpio${gpio}/value`
	if [ "$?" == "0" ]
	then
		echo $value
	else
		echo "-1"
	fi
}

# Writes specified GPIO
#
# @param $1 - GPIO number
#        $2 - < 0 | 1 >
#
# @return - 0 on success; 1 on error;
gpio_write_tlt() {
	local gpio=$1
	local value=$2
	local ret=""
	ret=`echo $value 2>/dev/null > /sys/class/gpio/gpio${gpio}/value`
	return $?
}

# Inverts specified GPIO
#
# @param $1 - GPIO number
#
# @return - 0 on success; 1 on error;
gpio_invert_tlt() {
	local gpio=$1
	local current_value
	local ret="1"
	
	current_value=$(gpio_read_tlt $gpio)
	
	if [ "$current_value" == "0" ]; then
		gpio_write_tlt $gpio 1
		ret="$?"
	fi
	
	if [ "$current_value" == "1" ]; then
		gpio_write_tlt $gpio 0
		ret="$?"
	fi
	
	return $ret
}

# Sets direction of specified GPIO
#
# @param $1 - GPIO number
#        $2 - direction < in | out >
#
# @return - 0 on success; 1 on error;
gpio_setdir_tlt() {
	local gpio=$1
	local dir=$2
	echo $dir 2>/dev/null > /sys/class/gpio/gpio${gpio}/direction
	return $?
}

# Gets direction of specified GPIO
#
# @param $1 - GPIO number
#
# @return - GPIO direction on success or
# -1 on error
gpio_getdir_tlt() {
	local gpio=$1
	local dir=""
	dir=`cat 2>/dev/null /sys/class/gpio/gpio${gpio}/direction`
	if [ "$?" == "0" ]
	then
		echo $dir
	else
		echo "-1"
	fi
}

gpio_init()
{
	if [ "`ls /sys/class/gpio/ | grep -c -e ^gpio$1$`" == "0" ]
	then
		echo $1 > /sys/class/gpio/export
	fi
	# GPIO direction seems to be correct after boot so no
	# direction set here.
	return 0
}

gpio_set()
{
	if ( gpio_init $1 && \
		echo 1 2>/dev/null > /sys/devices/virtual/gpio/gpio$1/value )
	then
		return 0
	fi
	return 1
}

gpio_unset()
{
	if ( gpio_init $1 && \
		echo 0 2>/dev/null > /sys/devices/virtual/gpio/gpio$1/value )
	then
		return 0
	fi
	return 1
}

gpio_get()
{
	local value=-1

	if ( gpio_init $1 && \
		echo in 2>/dev/null > /sys/class/gpio/gpio$1/direction )
	then
		value=`cat 2>/dev/null /sys/devices/virtual/gpio/gpio$1/value`
	fi
	echo "$value"
}

###
