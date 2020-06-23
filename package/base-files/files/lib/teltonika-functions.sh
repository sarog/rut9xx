#!/bin/sh

. /lib/functions.sh

# This script contains general Teltonika-specific
# bash functions to be included in other scripts
# in order to avoid multiple funtion definitions.

######################################
# USB related functions.
######################################

#Known modems
TELIT="1BC7:0021"
TELIT_LTE="1BC7:1201"
TELIT_LTE_V2="1BC7:0036"
HUAWEI="12D1:1404"
HUAWEI_LTE="12D1:1573"
HUAWEI_TD_LTE="12D1:15C1"
HUAWEI_ME906S="12D1:15C3"
SIERRA="1199:68C0"
QUECTEL="05C6:9215"
QUECTEL_EC25="2C7C:0125"
ZMTEL="258D:2000"
QUECTEL_UC20="05C6:9003"

# Checks external usb device and returns
# it's VID:PID.
#
# @return - VID:PID if found,
# "none" not attached, -1 error.
get_ext_vidpid_tlt() {
	local extvidpid=`lsusb | sed 'y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/' | \
	grep -E "($TELIT|$TELIT_LTE|$TELIT_LTE_V2|$HUAWEI|$HUAWEI_LTE|$HUAWEI_TD_LTE|$HUAWEI_ME906S|$SIERRA|$QUECTEL|$QUECTEL_EC25|$ZMTEL|$QUECTEL_UC20)" | \
	awk -F " " '{ print $6}'`
	if [ "$?" == "0" ]
	then
		echo $extvidpid
	else
		echo "-1"
	fi
}

get_usb_ext_vidpid_tlt() {
	local extvidpid=`lsusb | sed 'y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/' | \
	grep -E "(12d1:1506)" | awk -F " " '{ print $6}'`
	if [ "$?" == "0" ]
	then
		echo $extvidpid
	else
		echo "-1"
	fi
}

# Returns modules VID:PID from UCI
get_vidpid_tlt() {
	echo `uci get -q system.module.vid`":"`uci get -q system.module.pid`
}

######################################
# General purpose functions.
######################################

# converts ordinary netmask to CIDR
#
# @param $1 - subnet mask "dot notion"
#
# @return - CIDR
mask2cidr_tlt() {
	nbits=0
	IFS=.
	for dec in $1 ; do
	case $dec in
		255) let nbits+=8;;
		254) let nbits+=7;;
		252) let nbits+=6;;
		248) let nbits+=5;;
		240) let nbits+=4;;
		224) let nbits+=3;;
		192) let nbits+=2;;
		128) let nbits+=1;;
		0);;
		*) echo "Error: $dec is not recognised"; exit 1
	esac
	done
	echo "$nbits"
}

#####################################
# uci related functions
#####################################

# Writes one config OPTION entry in provided
# uci config file.
#
# @param $1 - config file name
# @param $2 - section type
# @param $3 - section name
# @param $4 - option name
# @param $5 - value
#
# @return - true on success, false otherwise.
# NOTE: if you provide non-existing section
# type or name, behaviuor is undefined.
tlt_uci_write_opt() {
	local config_name=$1
	local section_type=$2
	local section_name=$3
	local option_name=$4
	local value=$5
	local ftest_cmd=`uci changes $config_name 2>&1`

	if [ "$config_name" == "" -o \
		"$section_type" == "" -o \
		"$section_name" == "" -o \
		"$option_name" == "" -o \
		"$value" == "" ]
	then
		return 1
	fi

	if [ "$ftest_cmd" = "uci: Entry not found" ]
	then
		touch /etc/config/$config_name
		uci add $config_name $section_type
		uci rename $config_name.@$section_type[-1]=$section_name
		uci set $config_name.$section_name.$option_name=$value
		uci commit $config_name
	else
		local ret_cmd0=`uci get $config_name.$section_name 2>&1`
		local ret_cmd1=`uci get $config_name.$section_name.$option_name 2>&1`
		# FIXME: issue with existing type/name
		if [ "$ret_cmd1" != "$value" ]
		then
			if [ "$ret_cmd0" = "uci: Entry not found" ]
			then
				uci add $config_name $section_type
				uci rename $config_name.@$section_type[-1]=$section_name
			fi
			uci set $config_name.$section_name.$option_name=$value
			uci commit $config_name
		fi
	fi
	return 0
}

tlt_get_iface_ipaddr() {
	ifconfig "$1" | grep "inet addr" | awk -F ' ' '{print $2}' | awk -F ':' '{print $2}'
}

tlt_get_wan_ipaddr() {
	local debug="1"

	NAME=`uci get network.wan.ifname`
	SECTION="wan"
	if [ "$NAME" == "3g-ppp" ]; then
		#3g-ppp gets its IP individually, it does not reflect in wan IP
		SECTION="ppp"
        elif [[ "`get_ext_vidpid_tlt`" == "$TELIT_LTE_V2" -a "$NAME" == "wwan0" ]]; then
                SECTION="ppp"
        elif [[ "`get_ext_vidpid_tlt`" == "$QUECTEL_EC25" -a "$NAME" == "wwan0" ]]; then
                SECTION="ppp"
	elif [ "$NAME" == "wwan0" ]; then
		SECTION="ppp_dhcp"
	elif [ "$NAME" == "usb0" ]; then
		IP=`curl --user "user:user" --anyauth http://192.168.0.1/cgi/extip`
		EXTERNAL=`echo "$IP" | awk -F"\." ' $0 ~ /^([0-9]{1,3}\.){3}[0-9]{1,3}$/ && $1 <=255 && $2 <= 255 && $3 <= 255 && $4 <= 255 '`
		if [ "$EXTERNAL" == "" ]; then
			return 1
		else
			echo "$EXTERNAL"
			return 0
		fi
	fi
	EXTERNAL=`. /lib/functions/network.sh; network_flush_cache; network_get_ipaddr ip "$SECTION"; echo $ip`
	if [ "$EXTERNAL" == "" ]; then
		return 1
	else
		echo "$EXTERNAL"
		return 0
	fi
}

tlt_get_wan2_ipaddr() {
	local debug="1"

	NAME=`uci get -q network.wan2.ifname`
	if [ $NAME ]; then
		SECTION="wan2"
		if [ "$NAME" == "3g-ppp" ]; then
			#3g-ppp gets its IP individually, it does not reflect in wan IP
			SECTION="ppp"
                elif [[ "`get_ext_vidpid_tlt`" == "$TELIT_LTE_V2" -a "$NAME" == "wwan0" ]]; then
                        SECTION="ppp"
                elif [[ "`get_ext_vidpid_tlt`" == "$QUECTEL_EC25" -a "$NAME" == "wwan0" ]]; then
                        SECTION="ppp"
		elif [ "$NAME" == "wwan0" ]; then
			SECTION="ppp_dhcp"
		fi
		EXTERNAL=`. /lib/functions/network.sh; network_flush_cache; network_get_ipaddr ip "$SECTION"; echo $ip`
		if [ "$EXTERNAL" == "" ]; then
			return 1
		else
			echo "$EXTERNAL"
			return 0
		fi
	else
		return 1
	fi
}

# Get wan section by type "mobile, wired, wifi" or by ifname
get_wan_section() {
	config_load "network"
	local section cfgtype ifname interfaces
	local type="$1"
	[ "$#" -ge 1 ] && shift
	local arg="$1"
	[ "$#" -ge 1 ] && shift

	if [ "$type" == "type" ]; then
		case "$arg" in
			mobile)
				interfaces="eth2 3g-ppp wwan0"
			;;
			wired)
				interfaces="eth1"
			;;
			wifi)
				interfaces="wlan0"
			;;
			*)
				return 0
			;;
		esac
	elif [ "$type" == "ifname" ]; then
		interfaces=$arg
	else
		return 0
	fi

	[ -z "$CONFIG_SECTIONS" ] && return 0
	for section in ${CONFIG_SECTIONS}; do
		config_get cfgtype "$section" TYPE
		[ "x$cfgtype" != "xinterface" ] && continue
		case "$section" in
			wan*)
				config_get ifname $section ifname
				for item in $interfaces
				do
					if [ "$item" == "$ifname" ]; then
						echo $section
						return 1
					fi
				done
			;;
		esac
	done
}

wan_section_enabled() {
	config_load "network"
	local wan_section=`get_wan_section $1 $2`
	local enabled
	if [ $wan_section ]; then
		config_get enabled $wan_section enabled "1"
		echo $enabled
	else
		echo "0"
	fi
}

tlt_wait_for_wan() {
	local instance=$1
	MULTIWAN_FILE="/tmp/.mwan/cache"
	if [ -n "$instance" ]; then
		TMP_FILE="/tmp/tlt_wait_for_wan_or_backupwan${instance}.tmp"
	else
		TMP_FILE="/tmp/tlt_wait_for_wan_or_backupwan.tmp"
	fi

	while [ true ]; do
		WAN="wan"
		if [ -e "$MULTIWAN_FILE" ]; then
			wan_fail=`cat /tmp/.mwan/cache | grep wan_fail_map | awk -F '"' '{print $2}'`
			if [ "$wan_fail" == "wan[x]" ]; then
				#wan2
				WAN="wan2"
			fi
		fi
		NAME=`uci get network.$WAN.ifname`
		if [ "$NAME" == "3g-ppp" ]; then
			#3g-ppp gets its IP individually, it does not reflect in wan IP
			SECTION="ppp"
		elif [[ "`get_ext_vidpid_tlt`" == "$TELIT_LTE_V2" -a "$NAME" == "wwan0" ]]; then
			SECTION="ppp"
		elif [[ "`get_ext_vidpid_tlt`" == "$QUECTEL_EC25" -a "$NAME" == "3g-ppp" -o "`get_ext_vidpid_tlt`" == "$QUECTEL_EC25" -a "$NAME" == "wwan0" ]]; then
			SECTION="ppp"
		elif [ "$NAME" == "wwan0" ]; then
			SECTION="ppp_dhcp"
		else
			SECTION="$WAN"
		fi
		EXTERNAL=`. /lib/functions/network.sh; network_flush_cache; network_get_ipaddr ip "$SECTION"; echo $ip`
		if [ "$EXTERNAL" == "" ]; then
			if ! [ -e "$TMP_FILE" ]; then
				if [ -n "$instance" ]; then
					logger -t "$instance" "Waiting for WAN IP to show up."
				else
					logger "Waiting for WAN IP to show up"
				fi
				touch "$TMP_FILE"
			fi
		else
			if [ -e "$TMP_FILE" ]; then
				if [ -n "$instance" ]; then
					logger -t "$instance" "Waiting for WAN IP: ${EXTERNAL}"
				else
					logger "Waiting for WAN IP: ${EXTERNAL}"
				fi
				rm -rf "$TMP_FILE"
			fi
			echo "$EXTERNAL"
			break
		fi
		sleep 5
	done

}
#Determinate IP address if it belong to WAN or LAN

check_this_ip() {
	local only_ip=`echo $1 | grep -o '[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}' | head -1 `
	local lan_ip=`uci -q get network.lan.ipaddr`
	local lan_mac=`uci -q get network.lan.netmask`
	local via=""
	if [ -n "$lan_ip" ] || [ -n "$lan_mac" ] || [ "$lan_ip" == "" ] || [ "$lan_mac" == "" ]; then
		lan_ip=`ifconfig br-lan | grep "inet addr" | awk -F ' ' '{print $2}' | awk -F ':' '{print $2}'`
		lan_mac=`ifconfig br-lan | grep "inet addr" | awk -F ' ' '{print $4}' | awk -F ':' '{print $2}'`
	fi
	local lan_ipcal=`/bin/ipcalc.sh $lan_ip $lan_mac | grep 'BROADCAST\|NETWORK'`
	local det_ipcal=`/bin/ipcalc.sh $only_ip $lan_mac | grep 'BROADCAST\|NETWORK'`
	if [ "$lan_ipcal" == "$det_ipcal" ]; then
		via="LAN"
	else
		via="WAN"
	fi
	echo $via
	if [ "$2" == "true" ]; then
		/usr/bin/eventslog -i -t EVENTS -n "SSH" -e "Password auth succeeded from $via $1"
	fi
	if [ "$2" == "false" ]; then
		/usr/bin/eventslog -i -t EVENTS -n "SSH" -e "Bad password attempt from $via $1"
	fi
	if [ "$2" == "nonexistent" ]; then
		/usr/bin/eventslog -i -t EVENTS -n "SSH" -e "Login attempt for nonexistent user from  $via $1"
	fi

}
get_active_wan() {
	#Use "IP" to get interface IP. Otherwise ifname will be returned.
	local response_type=$1
	local multiwan=`uci -q get multiwan.config.enabled`
	if [ "$multiwan" = "1" ]; then
		if [ -f /tmp/.mwan/cache ]; then
			primary_wan=`cat /tmp/.mwan/cache | grep 'wan_if_map' | awk -F '[' '{ print $2 }' | awk -F ']' '{ print $1 }'`
			secondary_wan=`cat /tmp/.mwan/cache | grep 'wan_if_map' | awk -F '[' '{ print $3 }' | awk -F ']' '{ print $1 }'`
			failed_wan=`cat /tmp/.mwan/cache  | grep 'wan_fail_map' | awk -F '"' '{ print $2 }' | awk -F '[' '{ print $1 }'`

			if [ "$failed_wan" = "wan" ]; then
				if [ "$response_type" = "IP" ]; then
					echo `/sbin/ifconfig $secondary_wan | grep 'inet addr' | cut -d: -f2 | awk '{print $1}'`
				else
					echo "$secondary_wan"
				fi
			elif [ "$failed_wan" = "wan2" -o "$failed_wan" = "" ]; then
				if [ "$response_type" = "IP" ]; then
					get_ip_addr=`/sbin/ifconfig $primary_wan | grep 'inet addr' | cut -d: -f2 | awk '{print $1}'`
				else
					echo "$primary_wan"
				fi
			fi
		fi
	else
		if [ "$response_type" = "IP" ]; then
			tlt_get_wan_ipaddr
		else
			echo `uci -q get network.wan.ifname`
		fi
	fi
}
