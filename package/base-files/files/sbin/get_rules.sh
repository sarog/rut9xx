#!/bin/sh

. /lib/functions.sh
. /lib/teltonika-functions.sh

TakeRules(){
	config_get enabled "$1" "enabled" "0"
	config_get action "$1" "action" "0"
	config_get volts "$1" "volts" "0"
	config_get state "$1" "state" "0"
	config_get analog_state "$1" "analog_state" "0"
	config_get signal "$1" "signal" "0"
	echo -e "enabled $enabled action $action volts $volts state $state analog_state $analog_state signal $signal"

}
echo -n "" >/tmp/get_rule
config_load snmpd
config_foreach TakeRules "rule"

# get_conf=`cat /tmp/get_rule`
# rm /tmp/get_rule
# echo $get_conf
