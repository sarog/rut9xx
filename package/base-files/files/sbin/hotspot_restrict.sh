#!/bin/sh
# (C) 2015 Teltonika

. /lib/teltonika-functions.sh
. /lib/functions.sh
. /usr/share/libubox/jshn.sh

ACTION=$1
ID=$2

if [ "$ACTION" == "set" ]; then
	uci set hotspot_scheduler."$ID".restricted=1
	uci commit "hotspot_scheduler"
elif [ "$ACTION" == "clear" ]; then
	uci set hotspot_scheduler."$ID".restricted=0
	uci commit "hotspot_scheduler"
fi

    #  hotspot perkrovimas
	coova_section=$1

	json_load "$(/bin/ubus call network.wireless status)"
	json_select "radio0"

	if json_is_a "interfaces" array; then
		__idx=1
		json_select "interfaces"
		config_load wireless

		while json_is_a "$__idx" object; do
			json_select "$((__idx++))"
			json_get_var section section
			json_get_var DHCPIF ifname
			hotspotid=`uci -q get  wireless.$section.hotspotid`
			json_select ".."
			
			if [ "$ACTION" == "set" -a "$ID" == "$hotspotid" ]; then
				ifname_to_logout=$DHCPIF
				response=`chilli_query -s /var/run/chilli.$ifname_to_logout.sock list | cut -d' ' -f1`
				response_length=${#response} 
				macs_count=0
				for token in $response
					do
						macs_count=$((macs_count+1))
					done

				if [ "$response_length" -ge 17 ]; then
					i=1
					while [[ $i -le $macs_count ]]
						do
							response_mac=$(echo $response | cut -d' ' -f$i)
							`chilli_query -s /var/run/chilli.$ifname_to_logout.sock logout $response_mac`
							i=$((i+1))
						done
				fi
			fi
			done
	fi