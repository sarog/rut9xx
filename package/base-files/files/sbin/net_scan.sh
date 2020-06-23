#!/bin/sh
# Copyright (C) 2015 Teltonika
. /lib/functions.sh
. /lib/functions/network.sh

network_flush_cache
TMP_FILE_WAN="/tmp/net_scan_wan.txt"
TMP_FILE_LAN="/tmp/net_scan_lan.txt"
TMP_FILE_WIFI="/tmp/net_scan_wifi.txt"
TMP_FILE_HOST_IP="/tmp/net_scan_host_ip.txt"
TMP_FILE_HOST_WAN_IP="/tmp/net_scan_hots_wan_ip.txt"
TMP_FILE_WAN_GW="/tmp/net_scan_wan_gw.txt"
CHECK_IPCALC="/bin/ipcalc.sh"
TMP_FILE_HOSTNAMES="/tmp/net_scan_hostnames.txt"
TMP_FILE_HOSTNAMES_WAN="/tmp/net_scan_hostnames_wan.txt"
#lan config
LAN_IFNAME="br-lan"
LAN_IP=`uci -q get network.lan.ipaddr`
LAN_NETMASK=`uci -q get network.lan.netmask`
#wan config
WAN_IFNAME=$(uci_get_state network wan ifname)
WAN_SECTION="wan"
#WAN_IP=`ifconfig $WAN_IFNAME | grep 'inet addr:' | awk '{print $2}' | awk '{split($0,a,/\:/); print a[2]}'`
if [ "$WAN_IFNAME" == "3g-ppp" ] || [ "$WAN_IFNAME" == "3g_ppp" ] || [ "$WAN_IFNAME" == "3g" ]; then
	WAN_SECTION="ppp"
elif [ "$WAN_IFNAME" == "wwan0" ]; then
	WAN_SECTION="ppp_dhcp"
fi
network_get_ipaddr WAN_IP $WAN_SECTION
#WAN_IP=`uci -q get network.wan.ipaddr`
WAN_NETMASK=`ifconfig $WAN_IFNAME | grep 'inet addr:' | awk '{print $4}' | awk '{split($0,a,/\:/); print a[2]}'`

show_help()
{
	echo "Displays manufacturer information"
	echo "Usage:    | $0 <PARAM> <PARAM 2>"
	echo "<PARAM>   | all, wan, lan, wifi, clean"
	echo "<PARAM 2> | -s, --show"
	echo "Example:  | $0 lan -s"
}

check_wan_error()
{
	if [ "$WAN_IFNAME" == "3g-ppp" ] || [ "$WAN_IFNAME" == "eth2" ] || [ "$WAN_IFNAME" == "usb0" ] || [ "$WAN_IFNAME" == "3g_ppp" ] || [ "$WAN_IFNAME" == "3g" ] || [ "$WAN_IFNAME" == "wwan0" ]; then
		#local mobile_gw_ip=`route -n | grep -w "$WAN_IFNAME" | grep -w "UG" | awk '{print $2}' | tail -1`
		local mobile_gw_ip
		network_get_gateway mobile_gw_ip $WAN_SECTION
		if [ ! "$mobile_gw_ip" == "" ]; then
			local mobile_gw_mac=`arping -w 1 -I $WAN_IFNAME -c 1 $mobile_gw_ip | grep "Unicast reply from" | awk '{print $4, $5}'`
			echo "$mobile_gw_mac" | sed 's/\[//g' | sed 's/\]//g'>$TMP_FILE_WAN_GW
		else
			echo "">$TMP_FILE_WAN_GW
		fi
		echo "Error: WAN: $WAN_IFNAME! Only Wired, Wi-Fi are available"
		WAN_ERROR="mobile"
		send_error_to_wan "$WAN_ERROR"
		
		return 1
	fi
	
	WAN_STATUS=`cat /sys/class/net/$WAN_IFNAME/operstate`
	local WAN_PREFIX=`ipcalc.sh $WAN_IP $WAN_NETMASK | grep "PREFIX=" | cut -d'=' -f2`
	#local WAN_IPV6=`ifconfig $WAN_IFNAME | grep inet6`
	if [ "$WAN_STATUS" == "down" ]; then
		echo "Error: Interface name: $WAN_IFNAME is $WAN_STATUS!"
		WAN_ERROR="down"
		send_error_to_wan "$WAN_ERROR"
		return 1
	fi	
	
	if [ "$WAN_IFNAME" == "" ] || [ "$WAN_IP" == "" ] || [ "$WAN_NETMASK" == "" ]; then
		echo "ERROR: Some of WAN arguments are empty!\n"
		WAN_ERROR="arg_empty"
		send_error_to_wan "$WAN_ERROR"
		return 1
	fi
	

	#if [ ! "$WAN_IPV6" == "" ]; then
	#	echo "Error: Network scanner do not support IPV6"
	#	WAN_ERROR="ipv6"
	#	send_error_to_wan "$WAN_ERROR"
	#	return 1
	#fi
	if [ "$WAN_PREFIX" -lt "24" ]; then
		echo "Error: WAN network range is to large - max (255)"
		WAN_ERROR="net_range_to_large"
		send_error_to_wan "$WAN_ERROR"
		return 1
	fi
	return 0
}
check_lan_error()
{
	if [ "$LAN_IFNAME" == "" ] && [ "$LAN_IP" == "" ] && [ "$LAN_NETMASK" == "" ]; then
		echo "ERROR: Some of LAN arguments are empty!\n"
		LAN_ERROR="arg_empty"
		send_error_to_lan "$LAN_ERROR"
		return 1
	fi
	LAN_STATUS=`cat /sys/class/net/eth0/operstate`
	local LAN_PREFIX=`ipcalc.sh $LAN_IP $LAN_NETMASK | grep "PREFIX=" | cut -d'=' -f2`
	local LAN_IPV6=`ifconfig $LAN_IFNAME | grep inet6`
	if [ "$LAN_STATUS" == "down" ]; then
		local iwwinfo=`iwinfo`
		if [ ! -n "$iwwinfo" ]; then
			echo "Error: Interface name: $LAN_IFNAME is $LAN_STATUS!"
			LAN_ERROR="down"
			send_error_to_lan "$LAN_ERROR"
			return 1
		fi
	fi
	if [ "$LAN_PREFIX" -lt "24" ]; then
		echo "Error: LAN network range is to large - max (255)"
		LAN_ERROR="net_range_to_large"
		send_error_to_lan "$LAN_ERROR"
		return 1
	fi 
	return 0
}
check_wifi_error()
{
	WIFI_ERROR=""
	iwinfo=`iwinfo`
	if [ ! -n "$iwinfo" ]; then
		WIFI_ERROR="wireless_down"
		if [ -f $TMP_FILE_WIFI ]; then
			rm -rf $TMP_FILE_WIFI
		fi
		echo "$WIFI_ERROR">>$TMP_FILE_WIFI
		return 1
	fi
	return 0
}
send_error_to_wan()
{
	if [ -f $TMP_FILE_WAN ]; then
		rm -rf $TMP_FILE_WAN
	fi
	echo "$WAN_ERROR">>$TMP_FILE_WAN
}

send_error_to_lan()
{
	if [ -f $TMP_FILE_LAN ]; then
		rm -rf $TMP_FILE_LAN
	fi
	echo "$1">>$TMP_FILE_LAN
}

get_all_map()
{
	if check_lan_error ;  then
		get_lan_address $1 
	fi
	if check_wan_error ; then
		get_wan_address $1 
	fi
	if check_wifi_error ; then
		get_dhcp_leasses_address $1 
	fi
}

clean_all_files()
{
	if [ -f $TMP_FILE_WAN ]; then
		rm -rf $TMP_FILE_WAN
	fi
	if [ -f $TMP_FILE_LAN ]; then
		rm -rf $TMP_FILE_LAN
	fi
	if [ -f $TMP_FILE_WIFI ]; then
		rm -rf $TMP_FILE_WIFI
	fi
	if [ -f $TMP_FILE_HOST_IP ]; then
		rm -rf $TMP_FILE_HOST_IP
	fi
	if [ -f $TMP_FILE_HOST_WAN_IP ]; then
		rm -rf $TMP_FILE_HOST_WAN_IP
	fi
	if [ -f $TMP_FILE_WAN_GW ]; then
		rm -rf $TMP_FILE_WAN_GW
	fi
	if [ -f $TMP_FILE_HOSTNAMES ]; then
		rm -rf $TMP_FILE_HOSTNAMES
	fi
	if [ -f $TMP_FILE_HOSTNAMES_WAN ]; then
		rm -rf $TMP_FILE_HOSTNAMES_WAN
	fi
}
get_lan_address()
{
	local Start_from=`dmesg | tail -1`
	local Start_from_formated="\\$Start_from"
	local Link_down="eth0: link down"
	
	local LAN_REZULT
	if [ -n "$1" ]; then
		LAN_REZULT=$1
	else
		LAN_REZULT="false"
	fi
	set -- $2
	if [ -f $TMP_FILE_LAN ]; then
		rm -rf $TMP_FILE_LAN
	fi
	if [ -f $TMP_FILE_HOSTNAMES ]; then
		rm -rf $TMP_FILE_HOSTNAMES
	fi
	if [ -f $TMP_FILE_HOST_IP ]; then
		rm -rf $TMP_FILE_HOST_IP
	fi
	touch $TMP_FILE_LAN
	x="$LAN_IP"
	oc1=${x%%.*}
	x=${x#*.*}
	oc2=${x%%.*}
	x=${x#*.*}
	oc3=${x%%.*}
	local LAN_THREE_IP_SYM="$oc1.$oc2.$oc3."
	local LAN_CALC_RANGE=`ipcalc.sh $LAN_IP $LAN_NETMASK | grep 'NETWORK\|BROADCAST' | awk '{split($0,a,/\./); print a[4] a[8]}' | tr "\n" " "`
	local LAN_CALC_START=`echo $LAN_CALC_RANGE | awk '{print $2}'`
	local LAN_CALC_END=`echo $LAN_CALC_RANGE | awk '{print $1}'`
	local LAN_END=$((LAN_CALC_END - 1))
	local LAN_START=$((LAN_CALC_START + 1))
	local LAN_START_ONE=$((LAN_CALC_START + 1))
	while [ "$LAN_START" -le "$LAN_END" ]
	do
		arping -w 1 -I $LAN_IFNAME -c 1 $LAN_THREE_IP_SYM$LAN_START | grep "Unicast reply from" | awk '{print $4, $5}' >>$TMP_FILE_LAN &
		LAN_START=$((LAN_START + 1))
	done
	
	P=`pidof arping`
	#local time_to_sleep = 1
	while [ "$P" != "" ]; do
		P=`pidof arping`
	done
	#Remove special characters
	rm -rf /tmp/outas.txt
	sed -i 's/\[//g' $TMP_FILE_LAN && sed -i 's/\]//g' $TMP_FILE_LAN
	cat $TMP_FILE_LAN | awk '{print $1}'>$TMP_FILE_HOST_IP
	
	if [ -s "$TMP_FILE_HOST_IP" ]; then
		nbtscan -f $TMP_FILE_HOST_IP | tail -n +5 | awk '{print $1, $5, $2}' | grep -v "$LAN_IP " >>$TMP_FILE_HOSTNAMES
	fi 
	# sutvarkom hostname fila, del mac 00:00:00:00:00:00:00
	if [ -s "$TMP_FILE_HOSTNAMES" ]; then
		LAN_Hostnames_check_mac=`cat $TMP_FILE_HOSTNAMES | awk '{print $1, $2}' | grep "00-00-00-00-00-00"`
		if [ "$LAN_Hostnames_check_mac" != "" ]; then
			LAN_Host_ip=`echo "$LAN_Hostnames_check_mac" | awk '{print $1}'`
			for each_line in $LAN_Host_ip ; do
				#patikrinti ar unikali eile jai ne trinam is hostnames
				LAN_Hostnames_ip_addr=`cat $TMP_FILE_LAN | awk '{print $1}' | grep $each_line | uniq -d`
				if [ "$LAN_Hostnames_ip_addr" == "" ]; then
					#pridedam hostname y i faila wan
					LAN_Format_full_hostname=`cat $TMP_FILE_LAN | grep $each_line`
					LAN_Hostnames_hostname=`cat $TMP_FILE_HOSTNAMES | grep $each_line | awk '{print $3}'`
					LAN_hostname_formated_text=`echo $LAN_Format_full_hostname $LAN_Hostnames_hostname`
					sed -i "s/$each_line.*/$LAN_hostname_formated_text/g" $TMP_FILE_LAN
				fi
				sed -i "/$each_line/d" $TMP_FILE_HOSTNAMES
			done
		fi
	fi 
	if [ -f $TMP_FILE_HOST_IP ]; then
		rm -rf $TMP_FILE_HOST_IP
	fi
	LAN_Hostnames_ip_addr=`cat $TMP_FILE_HOSTNAMES | awk '{print $1}'`
	for line in $LAN_Hostnames_ip_addr ; do
		LAN_Format_mac_address=`cat $TMP_FILE_HOSTNAMES | grep $line`
		LAN_A_elem=`echo $LAN_Format_mac_address| awk '{print $1}'`
		LAN_B_elem=`echo $LAN_Format_mac_address | awk '{print $2}' | sed 's/\-/:/g'`
		LAN_C_elem=`echo $LAN_Format_mac_address | awk '{print $3}'`
		LAN_nbtscan_one_line=`echo $LAN_A_elem $LAN_B_elem $LAN_C_elem`
		sed -i "s/$line.*/$LAN_nbtscan_one_line/g" $TMP_FILE_LAN
	done
	#-------------------------------------------------
	#-----Include own IP and MAC, Hostname address----
	#-------------------------------------------------
	#fukcija ideti
	
	#check for duplicated ip address or mac
	#IP
	iiwinfo=`iwinfo`
	if [ -n "$iiwinfo" ]; then
		if [ -f $TMP_FILE_WIFI ]; then
			rm -rf $TMP_FILE_WIFI
		fi
		get_dhcp_leasses_address
		if [ -f $TMP_FILE_WIFI ]; then
			LAN_wifi_ip_addr=`cat $TMP_FILE_WIFI | awk '{print $1, $2}'`
			#remove fromn lan file this lines
			for LAN_wifi_info in $LAN_wifi_ip_addr ; do
				sed -i "/$LAN_wifi_info/d" $TMP_FILE_LAN
			done
			cat $TMP_FILE_WIFI>>$TMP_FILE_LAN 
		fi
	fi
	LAN_IP_duplicate=`cat $TMP_FILE_LAN | awk '{print $1}' | sort | uniq -d`
	if [ -n "$LAN_IP_duplicate" ]; then
		for ipaddress in $LAN_IP_duplicate ; do
			local Ip_dup_count=`cat $TMP_FILE_LAN | awk {'print $1'} | grep $ipaddress | uniq -c | awk '{print $1}'`
			sed -i "/^$ipaddress/ s/$/ IP-Dup_$Ip_dup_count/" $TMP_FILE_LAN
		done
		
	fi
	#MAC
	LAN_MAC_duplicate=`cat $TMP_FILE_LAN | awk '{print $2}' | sort | uniq -d`
	if [ -n "$LAN_MAC_duplicate" ]; then
		for macddress in $LAN_MAC_duplicate ; do
			LAN_MAC_dup_count=`cat $TMP_FILE_LAN | awk {'print $2'} | grep $macddress | uniq -c | awk '{print $1}'`
			LAN_asc=`awk '/'$macddress'/{print $0 " MAC-Dup_'$LAN_MAC_dup_count'"; next}1' $TMP_FILE_LAN`
			echo "$LAN_asc">$TMP_FILE_LAN
		done
	fi
	#Check if link was down
	local Read_dmesg_all=$(dmesg | tail -100 | grep -A 100 "$Start_from_formated" | grep -cim1 "$Link_down")
	if [ "$Read_dmesg_all" == "1" ]; then
		#Link was down;
		rm -rf $TMP_FILE_LAN
		echo "eth0: link down scan">$TMP_FILE_LAN
	elif [ "$Read_dmesg_all" == "0" ]; then
		#Everything is ok;
		echo "Everything is ok"
	else
		#Error: Read_dmesg_all;
		rm -rf $TMP_FILE_LAN
		echo "Error: dmesg_reader">$TMP_FILE_LAN
	fi
	#Check <PARAM>, print rezult
	if [ "$LAN_REZULT" == "--show" ] || [ "$LAN_REZULT" == "-s" ]; then
		if [ -f $TMP_FILE_LAN ]; then
			echo "--------------------LAN Information------------------------------"
			echo "| IP address | MAC address | Hostname/Duplication"
			cat $TMP_FILE_LAN
		fi
	fi
}
get_wan_address()
{
	local Start_from=`dmesg | tail -1`
	local Start_from_formated="\\$Start_from"
	local Link_down="$IFNAME link down"
	if [ ! -f $1 ]; then
		WAN_REZULTS=$1
	else
		WAN_REZULTS="false"
	fi
	set -- $2
	if [ -f $TMP_FILE_WAN ]; then
		rm -rf $TMP_FILE_WAN
	fi
	if [ -f $TMP_FILE_HOSTNAMES ]; then
		rm -rf $TMP_FILE_HOSTNAMES
	fi
	if [ -f $TMP_FILE_HOST_WAN_IP ]; then
		rm -rf $TMP_FILE_HOST_WAN_IP
	fi
	touch $TMP_FILE_WAN
	xx="$WAN_IP"
	occ1=${xx%%.*}
	xx=${xx#*.*}
	occ2=${xx%%.*}
	xx=${xx#*.*}
	occ3=${xx%%.*}
	local WAN_THREE_IP_SYM="$occ1.$occ2.$occ3."
	local WAN_CALC_RANGE=`ipcalc.sh $WAN_IP $WAN_NETMASK | grep 'NETWORK\|BROADCAST' | awk '{split($0,a,/\./); print a[4] a[8]}' | tr "\n" " "`
	local WAN_CALC_START=`echo $WAN_CALC_RANGE | awk '{print $2}'`
	local WAN_CALC_END=`echo $WAN_CALC_RANGE | awk '{print $1}'`
	local WAN_END=$((WAN_CALC_END - 1))
	local WAN_START=$((WAN_CALC_START + 1))
	local WAN_START_ONE=$((WAN_CALC_START + 1))
	while [ "$WAN_START" -le "$WAN_END" ]
	do
		arping -w 1 -I $WAN_IFNAME -c 1 $WAN_THREE_IP_SYM$WAN_START | grep "Unicast reply from" | awk '{print $4, $5}' >>$TMP_FILE_WAN &
		WAN_START=$((WAN_START + 1))
	done
	#Wait for arping finish
	WAN_P=`pidof arping`
	while [ "$WAN_P" != "" ]; do
		WAN_P=`pidof arping`
	done
	#tr [a-z] [A-Z] < $
	#Remove special characters
	sed -i 's/\[//g' $TMP_FILE_WAN && sed -i 's/\]//g' $TMP_FILE_WAN
	cat $TMP_FILE_WAN | awk '{print $1}'>$TMP_FILE_HOST_WAN_IP
	if [ -s "$TMP_FILE_HOST_WAN_IP" ]; then
		nbtscan -f $TMP_FILE_HOST_WAN_IP | tail -n +5 | awk '{print $1, $5, $2}' | grep -v "$WAN_IP " >>$TMP_FILE_HOSTNAMES_WAN
	fi 
	# sutvarkom hostname fila, del mac 00:00:00:00:00:00:00
	if [ -s "$TMP_FILE_HOSTNAMES_WAN" ]; then
		WAN_Hostnames_check_mac=`cat $TMP_FILE_HOSTNAMES_WAN | awk '{print $1, $2}' | grep "00-00-00-00-00-00"`
		if [ "$WAN_Hostnames_check_mac" != "" ]; then
			WAN_Host_ip=`echo "$WAN_Hostnames_check_mac" | awk '{print $1}'`
			for each_line in $WAN_Host_ip ; do
				#patikrinti ar unikali eile jai ne istrinam is hostnames
				WAN_Hostnames_ip_addr=`cat $TMP_FILE_WAN | awk '{print $1}' | grep $each_line | uniq -d`
				if [ "$WAN_Hostnames_ip_addr" == "" ]; then
					#pridedam hostname y i faila wan
					WAN_Format_full_hostname=`cat $TMP_FILE_WAN | grep $each_line`
					WAN_Hostnames_hostname=`cat $TMP_FILE_HOSTNAMES_WAN | grep $each_line | awk '{print $3}'`
					WAN_hostname_formated_text=`echo $WAN_Format_full_hostname $WAN_Hostnames_hostname`
					sed -i "s/$each_line.*/$WAN_hostname_formated_text/g" $TMP_FILE_WAN
				fi
				sed -i "/$each_line/d" $TMP_FILE_HOSTNAMES_WAN
			done
		fi
	fi
	if [ -f $TMP_FILE_HOST_WAN_IP ]; then
		rm -rf $TMP_FILE_HOST_WAN_IP
	fi
	Hostnames_ip_addr=`cat $TMP_FILE_HOSTNAMES_WAN | awk '{print $1}'`
	for line in $Hostnames_ip_addr ; do
		Format_mac_address=`cat $TMP_FILE_HOSTNAMES_WAN | grep $line`
		A_elem=`echo $Format_mac_address| awk '{print $1}'`
		B_elem=`echo $Format_mac_address | awk '{print $2}' | sed 's/\-/:/g'`
		C_elem=`echo $Format_mac_address | awk '{print $3}'`
		nbtscan_one_line=`echo $A_elem $B_elem $C_elem`
		sed -i "s/$line.*/$nbtscan_one_line/g" $TMP_FILE_WAN
	done
	
	#check for duplicated ip address or mac
	#IP
	IP_duplicate=`cat $TMP_FILE_WAN | awk '{print $1}' | sort | uniq -d`
	if [ -n "$IP_duplicate" ]; then
		for ipaddress in $IP_duplicate ; do
			Ip_dup_count=`cat $TMP_FILE_WAN | awk {'print $1'} | grep $ipaddress | uniq -c | awk '{print $1}'`
			sed -i "/^$ipaddress/ s/$/ IP-Dup_$Ip_dup_count/" $TMP_FILE_WAN
		done
	fi
	#MAC Workaround
	MAC_duplicate=`cat $TMP_FILE_WAN | awk '{print $2}' | sort | uniq -d`
	if  [ -n "$MAC_duplicate" ]; then
		for macddress in $MAC_duplicate ; do
			MAC_dup_count=`cat $TMP_FILE_WAN | awk {'print $2'} | grep $macddress | uniq -c | awk '{print $1}'`
			asc=`awk '/'$macddress'/{print $0 " MAC-Dup_'$MAC_dup_count'"; next}1' $TMP_FILE_WAN`
			echo "$asc">$TMP_FILE_WAN
		done
	fi
	# Default gateway
	local default_gw=$(route -n | grep "$WAN_IFNAME" | awk '{print $2}' | grep -v "0.0.0.0" | tail -1)
	echo "default_gw = $default_gw"
	if [ "$default_gw" != "" ]; then
		cat $TMP_FILE_WAN>>/tmp/vienas.txt
		local default_line=`cat $TMP_FILE_WAN | grep -w $default_gw | tail -1`
		
		echo "default_line = $default_line"
		if [ "$default_line" != "" ]; then
			local x=`cat $TMP_FILE_WAN | awk "/$default_gw/{ print NR; exit }" `
			if [ "$x" != "" ]; then
				sed -ie "$x d" $TMP_FILE_WAN
			fi
			echo "$default_line">$TMP_FILE_WAN_GW
		fi
	fi 
	#Check if link was down
	local Read_dmesg_all=$(dmesg | tail -100 | grep -A 100 "$Start_from_formated" | grep -cim1 "$Link_down")
	if [ "$Read_dmesg_all" == "1" ]; then
		#Link was down;
		rm -rf $TMP_FILE_WAN
		echo "WAN: link down scan">$TMP_FILE_WAN
	elif [ "$Read_dmesg_all" == "0" ]; then
		#Everything is ok;
		echo "Everything is ok"
	else
		#Error: Read_dmesg_all;
		rm -rf $TMP_FILE_WAN
		echo "Error: dmesg_reader">$TMP_FILE_WAN
	fi
	if [ $WAN_REZULTS == "--show" ] || [ $WAN_REZULTS == "-s" ]; then
		if [ -f $TMP_FILE_WAN ]; then
			echo "------------------- WAN Information------------------------------"
			echo "| IP address | MAC address | Hostname/Duplication "
			cat $TMP_FILE_WAN
		fi
	fi
}
get_dhcp_leasses_address()
{	
	iwinfo=`iwinfo`
	if [ -n "$iwinfo" ]; then
		if [ ! -f $1 ]; then
			WIFI_REZULT=$1
		else
			WIFI_REZULT="false"
		fi
		set -- $2
		if [ -f $TMP_FILE_WIFI ]; then
			rm -rf $TMP_FILE_WIFI
		fi
		touch $TMP_FILE_WIFI
		for interface in `iw dev | grep Interface | cut -f 2 -s -d" "` ; do
			maclist=`iw dev $interface station dump | grep Station | cut -f 2 -s -d" "`
			for mac in $maclist ; do
				ip="UNKN"
				host=""
				ip=`cat /tmp/dhcp.leases | cut -f 2,3,4 -s -d" " | grep $mac | cut -f 2 -s -d" "`
				host=`cat /tmp/dhcp.leases | cut -f 2,3,4 -s -d" " | grep $mac | cut -f 3 -s -d" "`
				if [ "$ip" != "" ] && [ "$mac" != "" ]; then
					echo "$ip $mac $host">>$TMP_FILE_WIFI
				fi
			done
		done
		if [ $WIFI_REZULT == "--show" ] || [ $WIFI_REZULT == "-s" ]; then
			if [ -f $TMP_FILE_WIFI ]; then
				echo "------------------ Wireless DHCP leasses-------------------------"
				echo "| IP address | MAC address | Hostname/Duplication"
				cat $TMP_FILE_WIFI
			fi
		fi
	fi
	
}
#Validation of parameters
if [ $# -lt 1 -o $# -gt 2 ] ; then
	show_help
	exit
fi
param=$1
#Action
case $param in
	"all")
		get_all_map $2
		exit
		;;	
	"lan")
		if check_lan_error ;  then
			get_lan_address $2
		fi
		exit
		;;	
	"wan")
		if check_wan_error ; then
			get_wan_address $2
		fi
		exit
		;;	
	"wifi")
		if check_wifi_error ; then
			get_dhcp_leasses_address $2
		fi
		exit
		;;
	"clean")
		clean_all_files
		exit
		;;
	*)
		echo "Parameter '$param' not supported"
		show_help
		exit
		;;
esac
exit 0
