#!/bin/sh
# source jshn shell library
. /usr/share/libubox/jshn.sh

INSTANCE=$(echo "$config" | cut -d"-" -f2 | cut -d"." -f1)
STATUS_FILE="/tmp/state/openvpn-$INSTANCE.info"

case $script_type in

        up)
                env | sed -n -e "
                /^foreign_option_.*=dhcp-option.*DNS /s//server=/p
                /^foreign_option_.*=dhcp-option.*DOMAIN /s//domain=/p
                " | sort -u > /tmp/dnsmasq.d/$dev.dns
		echo "strict-order" >> /tmp/dnsmasq.d/$dev.dns

		# generating json data
		json_init

		json_add_string "name" "$INSTANCE"
		json_add_string "ip" "$ifconfig_local"
		json_add_string "time" "$daemon_start_time"

		json_dump > "$STATUS_FILE"
                ;;
        down)
                rm /tmp/dnsmasq.d/$dev.dns 2> /dev/null
		rm "$STATUS_FILE" 2> /dev/null
                ;;
esac

/etc/init.d/dnsmasq reload &

