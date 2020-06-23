#!/bin/sh

DEBUG="1"
NAME="NCM.SH"

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

#Debug
debug(){
	if [ "$DEBUG" == "1" ]; then
		logger -t "$NAME" "$1"
	fi
}

proto_ncm_init_config() {
	no_device=1
	available=1
	proto_config_add_string "device:device"
	proto_config_add_string apn
	proto_config_add_string auth
	proto_config_add_string enabled
	proto_config_add_string username
	proto_config_add_string password
	proto_config_add_string auth_mode
	proto_config_add_string pincode
	proto_config_add_string delay
	proto_config_add_string mode
	proto_config_add_string pdptype
	proto_config_add_boolean ipv6
	proto_config_add_boolean ifname
	proto_config_add_string method
	proto_config_add_string mtu
	proto_config_add_int metric
}

proto_ncm_setup() {
	local interface="$1"

	local manufacturer initialize setmode connect devname devpath authenticate cid ppp_name

	local device apn auth enabled username password auth_mode pincode delay mode pdptype ipv6 ifname method json_cid mtu
	json_get_vars device apn auth enabled username password auth_mode pincode delay mode pdptype ipv6 ifname method mtu metric

	if [ "${device#*usb}" != "$device" ]; then
		ppp_name="ppp_usb"
	else
		ppp_name="ppp"
		# reikalinga del APN kad pasikeistu IP adresai
		# daznai buna like Acgatt=0 del to nesudaro pas kliendus duomenu konekcijos.
                attach=`gsmctl -A 'AT+CGATT?' | awk -F ' ' '{print $2}'`
                if [ "$attach" != "1" ]; then
			gsmctl -A 'AT+CGATT=1'
			sleep 2
		else
			gsmctl -A 'AT+CGATT=0'
			sleep 5
			gsmctl -A 'AT+CGATT=1'
			sleep 2
                fi

	fi

	if [ -z "$method" ]; then
		method=$(uci -q get network.$ppp_name.method)
	fi

	if [ "$enabled" != "1" ] && [ ! -f "/tmp/mobileon" ]; then
		ifdown $ppp_name
		return 0
	fi

	ipv6=1

	if [ "$ipv6" = 0 ]; then
		ipv6=""
	else
		ipv6=1
	fi

	[ -n "$pdptype" ] && {
		if [ "$pdptype" == "1" ]; then
			pdptype="IP"
		fi
	}

	[ -z "$pdptype" ] && {
		if [ -n "$ipv6" ]; then
			pdptype="IPV4V6"
		else
			pdptype="IP"
		fi
	}

	[ -n "$ctl_device" ] && device=$ctl_device

	[ -n "$device" ] || {
		echo "No control device specified"
		proto_notify_error "$interface" NO_DEVICE
		proto_set_available "$interface" 0
		return 1
	}
	[ -e "$device" ] || {
		echo "Control device not valid"
		proto_set_available "$interface" 0
		return 1
	}

	[ -n "$ifname" ] || {
		echo "The interface could not be found."
		proto_notify_error "$interface" NO_IFACE
		proto_set_available "$interface" 0
		return 1
	}

	[ -n "$delay" ] && sleep "$delay"

	#manufacturer=`gcom -d "$device" -s /etc/gcom/getcardinfo.gcom | awk '/Manufacturer/ { print tolower($2) }'`
	if [ "${device#*usb}" != "$device" ]; then
		manufacturer="huawei"
	else
		manufacturer="telit"
	fi

	# [ $? -ne 0 ] && {
	# 	echo "Failed to get modem information"
	# 	proto_notify_error "$interface" GETINFO_FAILED
	# 	return 1
	# }

	json_load "$(cat /etc/gcom/ncm.json)"
	json_select "$manufacturer"
	[ $? -ne 0 ] && {
		echo "Unsupported modem"
		proto_notify_error "$interface" UNSUPPORTED_MODEM
		proto_set_available "$interface" 0
		return 1
	}

if [ "${device#*usb}" != "$device" ]; then
	json_get_values initialize initialize
	debug "initialize: $initialize"
	for i in $initialize; do
		debug "initialize: $i"
		eval COMMAND="$i" gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
			echo "Failed to initialize modem"
			proto_notify_error "$interface" INITIALIZE_FAILED
			return 1
		}
	done

	[ -n "$pincode" ] && {
		PINCODE="$pincode" gcom -d "$device" -s /etc/gcom/setpin.gcom || {
			echo "Unable to verify PIN"
			proto_notify_error "$interface" PIN_FAILED
			proto_block_restart "$interface"
			return 1
		}
	}

	[ -n "$mode" ] && {
		json_select modes
		json_get_var setmode "$mode"
		COMMAND="$setmode" gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
			echo "Failed to set operating mode"
			proto_notify_error "$interface" SETMODE_FAILED
			return 1
		}
		json_select ..
	}

	json_get_vars connect
	eval COMMAND="$connect" gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
		echo "Failed to connect"
		proto_notify_error "$interface" CONNECT_FAILED
		return 1
	}

	echo "Connected, starting DHCP on $ifname"
	debug "proto_init_update $ifname 1"
	proto_init_update "$ifname" 1
	debug "proto_send_update $interface"
	proto_send_update "$interface"

	json_init
	json_add_string name "${interface}_4"
	json_add_string ifname "@$interface"
	json_add_string proto "dhcp"
	json_add_int metric $metric
	ubus call network add_dynamic "$(json_dump)"

	[ -n "$ipv6" ] && {
		json_init
		json_add_string name "${interface}_6"
		json_add_string ifname "@$interface"
		json_add_string proto "dhcpv6"
		json_add_int metric $metric
		json_add_string extendprefix 1
		ubus call network add_dynamic "$(json_dump)"
	}

else

	local netstate=`/usr/sbin/gsmctl -g`
	if [ -z "$netstate" ]; then
		#Retry if no answer was received
		sleep 1
		netstate=`/usr/sbin/gsmctl -g`
	fi

	count="0"
	retry="5"

	numeric=`uci get -q network.ppp.numeric`

	while [ "$netstate" != "registered (home)" ] && [ "$netstate" != "registered (roaming)" ];
	do
		if [ "$numeric" != "" ]; then
			/usr/sbin/gsmctl -A AT+COPS=2
			sleep 2
			/usr/sbin/gsmctl -A "AT+COPS=1,2,\"$numeric\""
			sleep 15
		else
			#jeigu automatinis operatoriaus parinkimas
			/usr/sbin/gsmctl -A AT+COPS=2
			sleep 2
			/usr/sbin/gsmctl -A AT+COPS=0
			sleep 15
		fi


# 		debug "netstate count == $count"
		netstate=`/usr/sbin/gsmctl -g`
		if [ "$netstate" == "denied" ] && [ "$count" == "$retry" ]; then
			/usr/sbin/gsmctl -A AT+CGDCONT=1
			/usr/sbin/gsmctl -A AT+CGDCONT=3
			/usr/sbin/gsmctl -A AT+CGDCONT=4
			debug "Netstate denied"
			/etc/init.d/modem restart
			count="0"
		fi
		count=$((count+1))

	done

	count="0"
	retry="5"
	context="/tmp/temp_context"
	debug "reading context"
	/usr/sbin/gsmctl -A AT+CGDCONT=1
	/usr/sbin/gsmctl -A AT+CGDCONT=3
	/usr/sbin/gsmctl -A AT+CGDCONT=4
	echo -e "1\n4" > $context
	conn_ok="0"

	vid=`uci -q get system.module.vid`
	pid=`uci -q get system.module.pid`

	while read -r line; do

# 		json_cid=`echo "$line" | awk -F ' ' '{print $2}' | awk -F ',' '{print $1}'`
		json_cid=$line
		debug "AAAAAAA json_cid==|$json_cid|"

		debug "initialize values"

		if [ -n "$apn" ]; then
				json_get_values initialize initialize
			cid="4"
			/usr/sbin/gsmctl -A AT+CGDCONT=1,\"$pdptype\"
		else
			cid="1"
		fi

		if [ "$vid" == "1BC7" ] && [ "$pid" == "0036" ]; then

			#pataisymas del kabuciu ivedino i password
			eval COMMAND="AT#SETHEXSTR=2" gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
				echo "Failed to initialize modem"
				proto_notify_error "$interface" INITIALIZE_FAILED
				return 1
			}
			eval COMMAND="AT+CGDCONT=$json_cid,\\\"$pdptype\\\",\\\"$apn\\\"" gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
				echo "Failed to initialize modem"
				proto_notify_error "$interface" INITIALIZE_FAILED
				return 1
			}
		else

			for i in $initialize; do
				eval COMMAND="$i" gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
					echo "Failed to initialize modem"
					proto_notify_error "$interface" INITIALIZE_FAILED
					return 1
				}
			done
		fi

		[ -n "$pincode" ] && {
			PINCODE="$pincode" gcom -d "$device" -s /etc/gcom/setpin.gcom || {
				echo "Unable to verify PIN"
				proto_notify_error "$interface" PIN_FAILED
				proto_block_restart "$interface"
				return 1
			}
		}

		if [ -z "$auth_mode" -o "$auth_mode" == "none" ]; then
			auth_mode="0"
		elif [ "$auth_mode" == "pap" ]; then
			auth_mode="1"
		elif [ "$auth_mode" == "chap" ]; then
			auth_mode="2"
		fi

		if [ "$vid" == "1BC7" ] && [ "$pid" == "0036" ]; then

			eval COMMAND='AT#PDPAUTH=$json_cid,$auth_mode,\"$username\",\"$password\"' gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
				echo "Failed to authenticate"
				proto_notify_error "$interface" INITIALIZE_FAILED
				return 1
			}
		else
			json_get_var authenticate authenticate
			eval COMMAND="$authenticate" gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
				echo "Failed to authenticate"
				proto_notify_error "$interface" INITIALIZE_FAILED
				return 1
			}
		fi

		ifconfig "$ifname" -arp up

		[ -n "$mode" ] && {
			json_select modes
			json_get_var setmode "$mode"
			COMMAND="$setmode" gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
				echo "Failed to set operating mode"
				proto_notify_error "$interface" SETMODE_FAILED
				return 1
			}
			json_select ..
		}

		#connecting mobile data
		while [ 1 ]; do
			wait="0"

			assign="AT#NCM=1,$json_cid"
			eval COMMAND="$assign" gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
				echo "Failed to assign NCM protocol"
			}

			active_pdp="AT+CGACT=1,$json_cid"
			eval COMMAND="$active_pdp" gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
				echo "Failed to active PDP NCM protocol"
			}

			if [ "$vid" == "1BC7" ] && [ "$pid" == "0036" ]; then
				connect="AT+CGDATA=\\\"M-RAW_IP\\\",$json_cid"
			else
				if [ -n "$apn" ]; then
					json_get_var connect connect
				else
					json_get_var connect connect_no_apn
				fi
			fi
			debug "try to connect..."

			eval COMMAND="$connect" gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
				echo "Failed to connect"
				proto_notify_error "$interface" CONNECT_FAILED
				count=$((count+1))
				debug "count after  == $count"
				if [ "$count" == "$retry" ]; then
					count="0"
					#isvalomas contextas po nepavykusiu konekciju setttinima
					debug "cleaning context"
					/usr/sbin/gsmctl -A AT+CGDCONT="$json_cid"
					debug "retry timeout, break loop"
					break;
				else
					debug "connection unsuccess, waiting before retry"
					wait="1"
					sleep 2
				fi
			}
			if [ "$wait" != "1" ]; then
				conn_ok="1"
				debug "connection successful exiting inside loop"
				break;
			fi
		done
		if [ "$conn_ok" == "1" ]; then
			debug "connection successful exiting main loop"
			break;
		fi
	done < $context

	rm "$context"
	if [ "$conn_ok" != "1" ]; then
		debug "connection not successful"
# 		/sbin/reboot
		return 1
	fi

	if [ -n "$mtu" ]; then
		ifconfig "$ifname" mtu "$mtu"
	fi

# 	IP=`gsmctl -A AT+CGPADDR="$cid" | awk -F '"' '{print $2}'`
	IP_1=`gsmctl -A AT+CGPADDR=1 | awk -F '"' '{print $2}'`
	IP_2=`gsmctl -A AT+CGPADDR=4 | awk -F '"' '{print $2}'`
	if [ ! -n "$IP_2" ]; then
            cid="1"
            IP=$IP_1
        elif [ "$IP_1" == "$IP_2" ]; then
            cid="1"
            IP=$IP_2
        else
            cid="4"
            IP=$IP_2
	fi

	echo "IP address: $IP"
	CGCONTRDP=`gsmctl -A AT+CGCONTRDP="$cid"`

	GW=`echo $CGCONTRDP | awk -F '"' '{print $6}'`
	NETMASK=`echo $CGCONTRDP | awk -F '"' '{print $4}' | awk -F '.' '{print $5"."$6"."$7"."$8}'`
	DNS=`echo $CGCONTRDP | awk -F '"' '{print $8}'`
	DNS2=`echo $CGCONTRDP | awk -F '"' '{print $10}'`
	echo "$GW"

	if [ "$method" = "bridge" ]; then
		proto_init_update "$ifname" 1 1
		proto_add_ipv4_address "$IP" "$NETMASK" "" "${GW:-2.2.2.2}"
		proto_send_update "$interface"
		pids=`ps w | grep -v grep | grep bridge_daemon.sh | awk -F ' ' '{print $1}'`
		kill -9 "$pids" &> /dev/null
		/usr/sbin/bridge_daemon.sh &

	else
		pids=`ps w | grep -v grep | grep bridge_daemon.sh | awk -F ' ' '{print $1}'`
		kill -9 "$pids" &> /dev/null

		ifconfig br-lan:0 down &> /dev/null
		ifconfig "$ifname" "$IP" netmask "$NETMASK" up
		route add default gw "$GW"
		/sbin/arp -s "$GW" 11:22:33:44:55:66

		proto_init_update "$ifname" 1 1
		proto_add_ipv4_address "$IP" "$NETMASK" "" "${GW:-2.2.2.2}"
		proto_add_ipv4_route 0.0.0.0 0 "$GW"
		proto_add_dns_server "$DNS"
		proto_add_dns_server "$DNS2"
		proto_send_update "$interface"

	fi

fi

}

proto_ncm_teardown() {
	local interface="$1"

	local manufacturer disconnect

	local device apn
	json_get_vars device apn

	echo "Stopping network"
	killall bridge_daemon.sh &> /dev/null

	#manufacturer=`gcom -d "$device" -s /etc/gcom/getcardinfo.gcom | awk '/Manufacturer/ { print tolower($2) }'`
	if [ "${device#*usb}" != "$device" ]; then
		manufacturer="huawei"
	else
		manufacturer="telit"
	fi

	# [ $? -ne 0 ] && {
	# 	echo "Failed to get modem information"
	# 	proto_notify_error "$interface" GETINFO_FAILED
	# 	return 1
	# }

	json_load "$(cat /etc/gcom/ncm.json)"
	json_select "$manufacturer" || {
		echo "Unsupported modem"
		proto_notify_error "$interface" UNSUPPORTED_MODEM
		return 1
	}

	if [ -n "$apn" ]; then
		json_get_values disconnect disconnect
	else
		json_get_values disconnect disconnect_no_apn
	fi


	if [ "${device#*usb}" != "$device" -a -n "$disconnect" ]; then
		COMMAND="$disconnect" gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
			echo "Failed to disconnect"
			proto_notify_error "$interface" DISCONNECT_FAILED
			return 1
		}
	else
		#Disconnect siunciamas i modem_cmd porta, nes modem_data portas gali neatsakyti, kai yra konekcija
		for i in $disconnect; do
			eval COMMAND="$i" /usr/sbin/gsmctl -A "$i" || {
				echo "Failed to disconnect"
				proto_notify_error "$interface" DISCONNECT_FAILED
				return 1
			}
		done
		ifconfig wwan0 "0.0.0.0" down
	fi

	proto_init_update "*" 0
	proto_send_update "$interface"
}
[ -n "$INCLUDE_ONLY" ] || {
	add_protocol ncm
}
