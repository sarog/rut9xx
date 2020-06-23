#!/bin/sh

apn=`uci -q get network.ppp.apn`
DNS_TC="/tmp/tmp_file/dnsmasq_pbridge.conf"
mkdir -p /tmp/tmp_file
mac=""

if [ "$(uci -q get network.ppp.method)" != "bridge" ]; then
	exit 0
fi


check(){
	IP=`gsmctl -A AT+CGPADDR=4 | awk -F '"' '{print $2}'`
	if [ ! -n "$IP" ]; then
	    IP=`gsmctl -A AT+CGPADDR=1 | awk -F '"' '{print $2}'`
	fi
	echo "$IP"

	CGCONTRDP=`gsmctl -A AT+CGCONTRDP=4`
	if [ $(echo $CGCONTRDP | grep -c "CME ERROR:") != '0' ] ; then
	    CGCONTRDP=`gsmctl -A AT+CGCONTRDP=1`
	fi


	GW=`echo $CGCONTRDP | awk -F '"' '{print $6}'`
	NETMASK=`echo $CGCONTRDP | awk -F '"' '{print $4}' | awk -F '.' '{print $5"."$6"."$7"."$8}'`
	DNS=`echo $CGCONTRDP | awk -F '"' '{print $8}'`
	DNS2=`echo $CGCONTRDP | awk -F '"' '{print $10}'`
	echo "$GW"
}


get_mac(){
	mac=`head -n 1 /tmp/dhcp.leases | awk '{print $2}'`
}

dhcp(){
	IPSHORT=`echo $IP | awk -F '.' '{print $1"."$2"."$3}'`

	rm -f "$DNS_TC"
	ifconfig br-lan:0 $IPSHORT.1 up
	echo "dhcp-option=6,$DNS,$DNS2" >> "$DNS_TC"
	echo "dhcp-option=3,$GW" >> "$DNS_TC"
	echo "dhcp-range=lan,$IP,$IP,$NETMASK,2m" >> "$DNS_TC"
	#/etc/init.d/dnsmasq restart
	/etc/init.d/network reload
	sleep 10
}

logger -s "startas"

while [ 1 ]
do
	check
	if [ -n "$IP" ];then
		logger -s "Found IP: $IP"
		dhcp
		old_ip=$IP
		break
	fi
	sleep 5
done

while [ 1 ]
do
	get_mac
	if [ -n "$mac" ]; then
		logger -s "Found mac: $mac"
		/usr/sbin/ebtables -t nat -D PREROUTING -i wwan0 -j dnat --to-destination $mac &> /dev/null
		/usr/sbin/ebtables -t nat -A PREROUTING -i wwan0 -j dnat --to-destination $mac
		old_mac=$mac
		break
	fi
	sleep 2;

done

while [ 1 ]
do

	check
	get_mac
	if [ "$old_ip" != "$IP" ]; then

		dhcp
		$old_ip=$IP
		logger -s "IP has changed."

	fi
	if [ "$old_mac" != "$mac" ] || ! `ebtables -t nat -L | grep -q "wwan0"`; then
		/usr/sbin/ebtables -t nat -D PREROUTING -i wwan0 -j dnat --to-destination $old_mac &> /dev/null
		/usr/sbin/ebtables -t nat -A PREROUTING -i wwan0 -j dnat --to-destination $mac
		echo "old $old_mac  new $mac"
		old_mac=$mac
		logger -s "Mac has changed."
	fi
	sleep 5

done
