#! /bin/sh
. /lib/functions.sh

set_rules(){
	config_get interface "$1" "interface" ""
	config_get allow_ip "$1" "allow_ip" ""
	IFS=$' '
	for j in $allow_ip; do
		iptables  -A zone_${interface}_input --proto "$protocol" -s "$j" --destination-port "$3" -j ACCEPT -m comment --comment "Enable_$4"
	done

	if [ "$interface" == "vpn" -o "$interface" == "lan" ]; then
		iptables -A zone_${interface}_input --proto "$protocol" --destination-port "$3" -j REJECT -m comment --comment "Enable_$4"
	fi
}

check_enable(){
if [ "$1" == "1" ]; then
	rs="$2"
	
	config_load "rs"
	config_get protocol "$rs" "protocol" "tcp"
	config_get mode "$rs" "mode" ""
	config_get type "$rs" "type" ""
	
	if [ "$mode" == "bidirect" -o "$mode" == "server" ]; then
		config_get port_listen "$rs" "port_listen" ""
		config_foreach set_rules "ip_filter_$rs" "$protocol" "$port_listen" "$rs"
	elif [ "$type" == "modbus" ]; then
		config_get port_listen "$rs" "modbus_port" ""
		config_foreach set_rules "ip_filter_$rs" "$protocol" "$port_listen" "$rs"		
	fi
fi
}

check_usb_filters(){
	filter_name=`uci -q get rs.$1.name`
	if [ "$2" == "$filter_name" ]; then
			set_rules "$1" "$4" "$3" "usb"
	fi
}

check_usb_device(){
	config_load "rs"
	#config_get name "$1" "name" ""
	config_get mode "$1" "mode" ""
	config_get type "$1" "type" ""
	config_get protocol "$1" "protocol" "tcp"
	
	if [ "$mode" == "bidirect" -o "$mode" == "server" ]; then
		config_get port "$1" "port_listen" ""
		config_foreach check_usb_filters "ip_filter_usb" "$1" "$port" "$protocol"
	elif [ "$type" == "modbus" ]; then
		config_get port "$1" "modbus_port" ""
		config_foreach check_usb_filters "ip_filter_usb" "$1" "$port" "$protocol"
	fi
}

check_all_usb_devices(){
	config_load "rs"
	config_foreach check_usb_device "usb" 
}

enable_232=`uci -q get rs.rs232.enabled`
check_enable "$enable_232" "rs232"
enable_485=`uci -q get rs.rs485.enabled`
check_enable "$enable_485" "rs485"
check_all_usb_devices
