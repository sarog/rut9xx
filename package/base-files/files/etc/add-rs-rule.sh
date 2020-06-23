#! /bin/sh

kk="0"
set_rules(){
	config_get interface "$1" "interface" ""
	config_get allow_ip "$1" "allow_ip" ""
	IFS=$' '
	for j in $allow_ip; do
		if [ "$interface" == "wan" ]; then
			iptables -I zone_${interface}_input --proto $protocol -s $j --destination-port $port_listen -j ACCEPT -m comment --comment "Enable_$4"
			kk="1"
		else
			iptables -A zone_${interface}_input --proto $protocol -s $j --destination-port $port_listen -j ACCEPT -m comment --comment "Enable_$4"
		fi
	done

	if [ "$interface" == "vpn" -o "$interface" == "lan" ]; then
		iptables -A zone_${interface}_input --proto $protocol --destination-port $port_listen -j REJECT -m comment --comment "Enable_$4"
	fi
	if [ "$interface" == "wan" ]; then
		if [ "$kk" == "0" ]; then
			iptables -I zone_${interface}_input --proto $protocol -s "0.0.0.0/0" --destination-port $port_listen -j ACCEPT -m comment --comment "Enable_$4"
			kk="1"
		fi
	fi
}

check_enable(){
if [ "$1" == "1" ]; then
	rs="$2"
	if [ "$rs" == "usb" ]; then 
		mode=`uci get -q usb_to_serial.rs232.mode`
		type=`uci get -q usb_to_serial.rs232.type`
		
		if [ "$mode" == "bidirect" -o "$mode" == "server" ]; then
			. /lib/functions.sh
			config_load "usb_to_serial"
			config_get protocol "rs232" "protocol" ""
			config_get port_listen "rs232" "port_listen" ""
			config_foreach set_rules "ip_filter_rs232" "$protocol" "$port_listen" "USB to serial"
		fi
	else
		mode=`uci get -q rs."$rs".mode`
		type=`uci get -q rs."$rs".type`
		if [ "$mode" == "bidirect" -o "$mode" == "server" -o "$type" == "modbus" ]; then
			. /lib/functions.sh
			config_load "rs"

			config_get protocol "$rs" "protocol" ""
			config_get port_listen "$rs" "port_listen" ""
			if [ "$type" == "modbus" ]; then
				port=`uci get -q rs."$rs".modbus_port`
				protocol="tcp"
				port_listen="$port"
			fi
			config_foreach set_rules "ip_filter_$rs" "$protocol" "$port_listen" "$rs"

			#if [ "$kk" == "0" -a "$type" != "modbus" ]; then
			#	iptables -I zone_wan_input --proto $protocol -s "0.0.0.0/0" --destination-port $port_listen -j ACCEPT -m comment --comment "Enable_$rs"
			#fi

		fi
	fi
fi
}

enable_232=`uci get -q rs.rs232.enabled`
check_enable "$enable_232" "rs232"
enable_485=`uci get -q rs.rs485.enabled`
check_enable "$enable_485" "rs485"
enable_usb=`uci get -q usb_to_serial.rs232.enabled`

check_enable "$enable_usb" "usb"
