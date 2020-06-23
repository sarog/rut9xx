#!/bin/sh

#
# Control bar LEDs
#
# (c) 2014 Teltonika

. /lib/led_functions.sh

show_options()
{
	printf "usage: $0 <number of leds>\n"
}

blink_leds()
{
	echo "timer" > "$LED_PATH/$led0/trigger"
	echo "timer" > "$LED_PATH/$led1/trigger"
	echo "timer" > "$LED_PATH/$led2/trigger"
	echo "timer" > "$LED_PATH/$led3/trigger"
	echo "timer" > "$LED_PATH/$led4/trigger"
	[ "$1" = "red" ]   && echo "timer" > "$LED_PATH/$ledr/trigger"
	[ "$1" = "green" ] && echo "timer" > "$LED_PATH/$ledg/trigger"
	exit 0
}

if [ -n "$1" ] && [ "$1" != "blink" ] && [ "$1" -gt 4 ]
then
	show_options
	exit 1
fi

all_init
all_off
led_off $ledr
led_off $ledg

[ -z "$1" ] && exit 0

[ "$1" = "blink" ] && blink_leds $2

[ "$1" -ge 0 ] && led_on $led0
[ "$1" -ge 1 ] && led_on $led1
[ "$1" -ge 2 ] && led_on $led2
[ "$1" -ge 3 ] && led_on $led3
[ "$1" -ge 4 ] && led_on $led4

exit 0

