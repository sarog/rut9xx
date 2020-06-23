#!/bin/sh

veiksmas="$1"
eventas="$2"
eventmark="$3"

device_id=`uci -q get hwinfo.hwinfo.serial`
device_mac=`ifconfig | grep 'br-lan' | awk -F ' ' '{print $5}'`
netmask="255.255.224.0"
router_ip=`gsmctl -p tun_rms`
server_ip=`/bin/ipcalc.sh $router_ip $netmask | grep NETWORK | cut -f2 -d= | cut -f1,2,3 -d.`
server_ip_full="$server_ip.1"
id=""

if [ "$eventas" == "Signal strength" ]
then
	if [ "$veiksmas" == "sendSMS" ]
	then
		id="${id}1:"
	else
		id="${id}2:"
	fi
elif [ "$eventas" == "SIM switch" ]
then
	if [ "$veiksmas" == "sendSMS" ]
	then
		id="${id}3:"
	else
		id="${id}4:"
	fi
elif [ "$eventas" == "Mobile data" ]
then
	id="${id}5:"
else
	exit 1
fi

case $eventmark in
	"Signal strength dropped below -113 dBm") id="${id}0" ;;
	"Signal strength dropped below -98 dBm") id="${id}1" ;;
	"Signal strength dropped below -93 dBm") id="${id}2" ;;
	"Signal strength dropped below -75 dBm") id="${id}3" ;;
	"Signal strength dropped below -60 dBm") id="${id}4" ;;
	"Signal strength dropped below -50 dBm") id="${id}5" ;;
	
	"SIM 1 to SIM 2") id="${id}6" ;;
	"SIM 2 to SIM 1") id="${id}7" ;;
	
	"SIM1") id="${id}8" ;;
	"SIM2") id="${id}9" ;;
	
	*) echo "ERROR in switch statement"
esac

komanda="curl -d '{\"v\": \"1\", \"dev\": [\"$device_id\",\"$device_mac\"], \"par\":{\"meth\": \"trap_v1\",\"id\":\"$id\",\"actions\":\"$veiksmas\",\"events\":\"$eventas\",\"event_marks\":\"$eventmark\"}}' http://$server_ip_full/alert_from_device/web.cgi;"

returnvalue=`eval $komanda`


