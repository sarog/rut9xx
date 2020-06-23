#!/bin/sh

#
# Helper functions for LED control
#
# (c) 2014 Teltonika

LED_PATH="/sys/class/leds"

led0="signal_bar0"
led1="signal_bar1"
led2="signal_bar2"
led3="signal_bar3"
led4="signal_bar4"
ledg="status_green"
ledr="status_red"

led_init()
{
	trigger_path="$LED_PATH/$1/trigger"
	if ! [ -f "$trigger_path" ]
	then
		echo "$0: file $trigger_path not found"
		logger -t $0 "file $trigger_path not found"
	fi
	
	echo "none" > $trigger_path
}

all_init()
{
	led_init $led0
	led_init $led1
	led_init $led2
	led_init $led3
	led_init $led4
}

gpio_init()
{
	if [ "`ls /sys/class/gpio/ | grep -c -e ^gpio$1$`" == "0" ]
	then
		echo $1 > /sys/class/gpio/export
	fi
	
	if [ "$(cat /sys/class/gpio/gpio$1/direction)" != "out" ]
	then
		echo "out" > /sys/class/gpio/gpio$1/direction
	fi
	
	return $?
}

led_on()
{
	bright_path="$LED_PATH/$1/brightness"
	echo "1" > $bright_path
}

led_off()
{
	bright_path="$LED_PATH/$1/brightness"
	echo "0" > $bright_path
}

all_off()
{
	led_off $led0
	led_off $led1
	led_off $led2
	led_off $led3
	led_off $led4
}

