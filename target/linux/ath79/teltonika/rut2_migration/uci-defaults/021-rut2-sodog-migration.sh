#!/bin/sh
. /lib/functions.sh
#copy usb_to_serial config to RS config

[ -f "/etc/config/teltonika" ] || return 0

option_cb() {
    local name="$1"
    local value="$2"
   
	if [ "$CONFIG_SECTION" = "rs232" ]; then
		uci set rs.rs232_usb.$name=$value		
	fi
}

handle_allowed_ips(){
	local value="$1"
	local new_cfg_name="$2"
	uci add_list rs.$new_usb_filter_section.allow_ip=$value
}

handle_usb_filters() {
	new_usb_filter_section=$(uci add rs ip_filter_usb)
    local config_name="$1"

   interface_name=$(uci get usb_to_serial.$config_name.interface)
   
   uci set rs.$new_usb_filter_section.interface=$interface_name
   uci set rs.$new_usb_filter_section.name="rs232_usb"

   config_list_foreach $config_name allow_ip handle_allowed_ips $new_usb_filter_section
}

uci set rs.rs232_usb=usb
uci set rs.rs232_usb.name="USB_device1"
config_load usb_to_serial
config_foreach handle_usb_filters ip_filter_rs232

adapter_id=`ls /dev/rs232_usb_* | head -n 1 | cut -c 16-`
if [ -n "$adapter_id" ]; then
		uci set rs.rs232_usb.id="$adapter_id"
fi

uci_commit rs
