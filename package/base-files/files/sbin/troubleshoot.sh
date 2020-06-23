#!/bin/sh

. /lib/teltonika-functions.sh

PACK_DIR="/tmp/troubleshoot/"
ROOT_DIR="${PACK_DIR}root/"
PACK_FILE="/tmp/troubleshoot.tar.gz"
LOG_FILE="${PACK_DIR}system.log"
TMP_LOG_FILE="/tmp/tmp_syslog.log"
USBDEV="$(get_ext_vidpid_tlt)"
CENSORED_STR="VALUE_REMOVED_FOR_SECURITY_REASONS"
WHITE_LIST="
dropbear.@dropbear[0].PasswordAuth
dropbear.@dropbear[0].RootPasswordAuth
luci.flash_keep.passwd
mosquitto.mqtt.password_file
openvpn.teltonika_auth_service.persist_key
openvpn.teltonika_auth_service.auth_user_pass
rpcd.@login[0].username
rpcd.@login[0].password
rpcd.@rms_login[0].username
rpcd.@rms_login[0].password
teltonika.sys.pass_changed
uhttpd.main.key
uhttpd.hotspot.key
"

generate_collectibles() {
	local log_file="$1"
	local sep="-------------------------------------------------------------"
	local iflist="`ls /sys/class/net/`"

	echo -e "${sep}\nSYSTEM INFORMATION\n${sep}" >> $log_file
	echo -e "\n[Firmware version]\n`cat /etc/version`" >> $log_file
	echo -e "\n[Bootloader version]\n`/sbin/mnf_info blver`" >> $log_file
	echo -e "\n[Time]\n`date`" >> $log_file
	echo -e "\n[Uptime]\n`uptime`" >> $log_file
	if [ -e /proc/version ] ; then
		echo -e "\n[Build string]\n`cat /proc/version`" >> $log_file
	fi
	if [ -e /proc/mtd ] ; then
		echo -e "\n[Flash partitions]\n`cat /proc/mtd`" >> $log_file
	fi
	if [ -e /proc/meminfo ] ; then
		echo -e "\n[Memory usage]\n`cat /proc/meminfo`" >> $log_file
	fi

	echo -e "\n[Filesystem usage statistics]\n`df -h`" >> $log_file
	echo -e "\n[Process list]\n`ps -w`" >> $log_file
	echo -e "\n[Kernel modules]\n`lsmod`" >> $log_file
	echo -e "\n[Device files]\n`ls -al /dev/`" >> $log_file

	echo -e "\n[Log dir]\n`ls -al /log/`" >> $log_file
	echo -e "\n[Interfaces]" >> $log_file
	for iface in $iflist ; do
		ifconfig $iface >> $log_file 2>/dev/null
		ip a sh dev $iface >> $log_file 2>/dev/null
		echo -e "\n" >> $log_file 2>/dev/null
	done
	echo -e "\n[Tunnels]\n`ip tun`" >> $log_file
	echo -e "\n[Bridges]\n`brctl show`" >> $log_file
	echo -e "\n[Switch configuration]\n`swconfig dev switch0 show`" >> $log_file
	echo -e "\n[Routing table]\n`route -n -e`" >> $log_file

	local dhcp_list_data
	dhcp_list_data=$(cat /tmp/dhcp.leases)
	if [[ -f /tmp/dhcp.leases && "$dhcp_list_data" != "" ]]
	then
		echo -e "\n[DHCP leases]\n $dhcp_list_data" >> $log_file
	else
		echo -e "\n[DHCP leases]\n no DHCP leases.." >> $log_file
	fi

	echo -e "\n[ARP Data]\n`cat /proc/net/arp`" >> $log_file

	echo -e	"\n[WIFI clients]\n`iw dev wlan0 station dump | grep Station | awk '{print $2}'`" >> $log_file

	if [ $(lsmod | grep -q ^ip_tables && echo $?)  ] ; then
		echo -e "\n[IPtables FILTER]\n`ip tun`" >> $log_file
		iptables -L -nv >> $log_file 2>/dev/null
		if [ $(lsmod | grep -q ^iptable_nat && echo $?)  ] ; then
			echo -e "\n[IPtables NAT]\n" >> $log_file
			iptables -t nat -L -nv >> $log_file 2>/dev/null
		fi
		if [ $(lsmod | grep -q ^iptable_mangle && echo $?)  ] ; then
			echo -e "\n[IPtables MANGLE]\n" >> $log_file
			iptables -t mangle -L -nv >> $log_file 2>/dev/null
		fi
		iptables-save >> $log_file 2>/dev/null
	fi

	if [ $(lsmod | grep -q ^ebtables && echo $?)  ] ; then
		echo -e "\n[EBtables FILTER]\n`ip tun`" >> $log_file
		ebtables -t filter -L --Lc >> $log_file 2>/dev/null
		if [ $(lsmod | grep -q ^ebtable_nat && echo $?)  ] ; then
			echo -e "\n[EBtables NAT]\n" >> $log_file
			ebtables -t nat -L --Lc >> $log_file 2>/dev/null
		fi
		if [ $(lsmod | grep -q ^ebtable_broute && echo $?)  ] ; then
			echo -e "\n[EBtables BROUTE]\n" >> $log_file
			ebtables -t broute -L --Lc >> $log_file 2>/dev/null
		fi
	fi

	echo -e "\n${sep}\nLOGGING INFORMATION\n${sep}" >> $log_file
	echo -e "\n[Dmesg]\n`dmesg`" >> $log_file

	#Different log location if we are logging to flash
	log_type=`uci get system.system.log_type`
	if [ "$log_type" = "file" ]
	then
		flash_file=`uci get system.system.log_file`
		if [ -z "$flash_file" ]; then
			#Use default file
			flash_file="/var/log/messages"
		fi
		echo -e "\n[Logread]\n`cat \"$flash_file\"`" >> $log_file

	else
		echo -e "\n[Logread]\n`logread`" >> $log_file
	fi

	echo -e "\n[GSM INFORMATION]" >> $log_file
	logread -f > $TMP_LOG_FILE &
	for cmd in connstate netstate imei model manuf serial revision imsi simstate pinstate signal cellid operator opernum conntype temp
		do
			echo -ne "$cmd:   \t" >> $log_file
			gsmctl --$cmd >> $log_file 2>&1
		done
	sleep 1
	killall logread

	echo -e "\n[GSMD log of gsmctl]\n`cat \"$TMP_LOG_FILE\"`" >> $log_file

	echo -e "\n[GSMD AT commands]\n" >> $log_file
	for cmd in AT+CGATT? AT+CGDCONT? AT+CGACT? AT#AUTOATT?
	do
		echo -ne "$cmd:   \t" >> $log_file
		gsmctl -A $cmd >> $log_file 2>&1
	done
	sleep 1

	echo -e "\n[GPIO information]\n" >> $log_file
	for name in SIM DOUT1 DOUT2 DIN1 DIN2 MON MRST SDCS RFINT
	do
		echo -ne "$name:	\t" >> $log_file
		/sbin/gpio.sh get $name >> $log_file 2>&1
	done
	local topology="0"
	if [ "$2" == "true" ]; then
		topology="1"
	fi
	echo -e "\n[NETWORK TOPOLOGY]\nEnabled: \t$topology\n" >> $log_file
	echo -e "\n[IPSEC STATUS]\n" >> $log_file
	ipsec statusall >> $log_file
}
generate_topology() {
	local topology="/tmp/network_topology"
	if [ -f $topology ]; then
		rm -rf $topology
	fi
	/sbin/net_scan.sh all -s>>$topology
	PID_P=`pidof net_scan.sh`
	while [ "$PID_P" != "" ]; do
		PID_P=`pidof net_scan.sh`
	done
}

generate_random_str() {
	local out=$(</dev/urandom tr -dc A-Za-z0-9 | head -c $1)
	local is_ascii=$(echo -ne "$out" | strings)

	if [ ${#is_ascii} -eq $1 ]; then
		echo "$out"
	fi
}

secure_config() {
	local values=$(uci -c "$ROOT_DIR/etc/config" show | grep -iE "(\.)(.*)(pass|psw|pasw|psv|pasv|key|secret|username)(.*)=" | grep -iE "((([A-Za-z0-9]|\_|\@|\[|\]|\-)*\.){2})(.*)(pass|psw|pasw|psv|pasv|key|secret|username)(.*)=")
	local tmp_file=$(generate_random_str 64)

	echo "$values" > "/tmp/$tmp_file"

	while read -r Line; do
		Line=${Line%%=\'*}
		echo "$WHITE_LIST" | grep -iqFx "$Line"

		if [ $? -eq 1 ]; then
			pass_list="$pass_list
${Line%%=\'*}"	#do not indent this line!
		fi
	done < "/tmp/$tmp_file"

	rm "/tmp/$tmp_file"
	echo "$pass_list" > "/tmp/$tmp_file"

	while read -r Line; do
		if [ -n "$Line" ]; then
			uci -c "$ROOT_DIR/etc/config" set "$Line=$CENSORED_STR"
		fi
	done < "/tmp/$tmp_file"

	uci -c "$ROOT_DIR/etc/config" commit

	rm "/tmp/$tmp_file"
}

generate_root() {
	mkdir $ROOT_DIR
	# export /etc
	mkdir $ROOT_DIR/etc
	cp -r /etc/config /etc/crontabs /etc/dropbear /etc/firewall.user /etc/group /etc/hosts /etc/inittab /etc/passwd /etc/profile /etc/rc.local /etc/shells /etc/sysctl.conf $ROOT_DIR/etc
	secure_config
	# export /tmp
	mkdir $ROOT_DIR/tmp
	#cp -r /tmp/.uci /tmp/TZ /tmp/dhcp.leases /tmp/etc /tmp/lock /tmp/log /tmp/overlay /tmp/resolv.conf /tmp/resolv.conf.auto /tmp/run /tmp/spool /tmp/state $ROOT_DIR/tmp
	#cp -rf /tmp/* $ROOT_DIR/tmp; rm -rf $ROOT_DIR/tmp/troubleshoot.tar.gz $ROOT_DIR/tmp/troubleshoot
	for file in `ls /tmp/ | grep -v troubleshoot`; do cp -rf /tmp/"$file" $ROOT_DIR/tmp ; done
	/usr/bin/eventslog -p -t EVENTS -d1 -f $PACK_DIR/system_eventsdb.log
	/usr/bin/eventslog -p -t connections -d1 -f $PACK_DIR/network_eventsdb.log
	/usr/bin/eventslog -p -t ALL -d1 -f $PACK_DIR/all_eventsdb.log
	if [ "$1" == "true" ]; then
		generate_topology
		cp -r /tmp/network_topology $PACK_DIR/network_topology
	fi
}

generate_package() {
	cd /tmp
	tar -czf $PACK_FILE troubleshoot >/dev/null 2>&1
	rm -r $PACK_DIR
	rm -f $TMP_LOG_FILE
}

init() {
	rm -r $PACK_DIR >/dev/null 2>&1
	rm $PACK_FILE >/dev/null 2>&1
	mkdir $PACK_DIR
}

init

if [ -n "$1" ]; then
	if [ "$1" == "topology" ]; then
		generate_collectibles $LOG_FILE "true"
		generate_root "true"
	fi
else
	generate_collectibles $LOG_FILE
	generate_root
fi
generate_package
