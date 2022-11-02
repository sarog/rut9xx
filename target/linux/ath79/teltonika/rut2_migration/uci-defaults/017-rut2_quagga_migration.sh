#!/bin/sh

. /lib/functions.sh
. /lib/functions/network.sh

[ -f "/etc/config/teltonika" ] || return 0

peer_count=1

config_cb() {
	local type="$1"
	local name="$2"
	option_cb() { return; }
	list_cb() { return; }

	case "$name" in
		default)
			uci_add frr bgp_instance main_instance
			option_cb() {
				local name="$1"
				local value="$2"
				uci_set frr main_instance "$name" "$value"
			}

			list_cb() {
				local name="$1"
				local value="$2"
				uci_add_list frr main_instance "$name" "$value"
			}
			uci_remove frr default
			;;

		general)
			uci_add frr bgp_general bgp
			option_cb() {
				local name="$1"
				local value="$2"
				uci_set frr bgp "$name" "$value"
			}
			uci_remove frr general
			;;
	esac

	case "$type" in
		ospf_network)
			option_cb() {
				local name="$1"
				local value="$2"
				[ "$name" = "area" ] && {
					uci_add frr ospf_area "$value"
					uci_set frr "$value" area "$value"
					uci_set frr "$value" enabled "1"
				}
			}
			;;
		peer)
			uci_set frr "$name" instance main_instance
			uci_rename frr "$name" "peer${peer_count}"
			peer_count=$((peer_count+1))
			;;
		rip)
			uci_rename frr rip rip_old
			uci_add frr rip_general rip
			option_cb() {
				local name="$1"
				local value="$2"
				uci_set frr rip "$name" "$value"
			}
			list_cb() {
				local name="$1"
				local value="$2"
				uci_add_list frr rip "$name" "$value"
			}
			uci_remove frr rip_old
			;;
	esac
}

mv /etc/config/quagga /etc/config/frr
config_load frr
uci_add frr eigrp_general 'eigrp'
uci_commit frr

exit 0
