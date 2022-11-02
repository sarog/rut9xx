#! /bin/sh
. /lib/functions.sh

add_rules(){
	local path port_scan hitcount syn_fin syn_rst x_max nmap_fin null_flags
	config_get path "$1" path

	[ "$basedir" = "$path" ] || return

	config_get port_scan    "$1" port_scan  "0"
	config_get seconds      "$1" seconds    "30"
	config_get hitcount     "$1" hitcount   "10"
	config_get syn_fin      "$1" syn_fin    "0"
	config_get syn_rst      "$1" syn_rst    "0"
	config_get x_max        "$1" x_max      "0"
	config_get nmap_fin     "$1" nmap_fin   "0"
	config_get null_flags   "$1" null_flags "0"

	[ "$port_scan" = "1" ] && {
		iptables -N zone_port_scan
		iptables -A zone_port_scan -p tcp -m conntrack --ctstate NEW -m recent --set
		iptables -A zone_port_scan -p tcp -m conntrack --ctstate NEW -m recent --update --seconds "$seconds" --hitcount "$hitcount" -j DROP
		iptables -t filter -I zone_wan_input -j zone_port_scan
		iptables -t filter -I zone_wan_forward -j zone_port_scan
	}

	[ "$syn_fin" = "1" ] && {
		iptables -t raw -I PREROUTING -p tcp --tcp-flags FIN,SYN FIN,SYN -j DROP
	}

	[ "$syn_rst" = "1" ] && {
		iptables -t raw -I PREROUTING -p tcp --tcp-flags SYN,RST SYN,RST -j DROP
	}

	[ "$x_max" = "1" ] && {
		iptables -t raw -I PREROUTING -p tcp --tcp-flags FIN,SYN,RST,PSH,ACK,URG FIN,PSH,URG -j DROP
	}

	[ "$nmap_fin" = "1" ] && {
		iptables -t raw -I PREROUTING -p tcp --tcp-flags FIN,SYN,RST,PSH,ACK,URG FIN -j DROP
	}
	[ "$null_flags" = "1" ] && {
		iptables -t raw -I PREROUTING -p tcp --tcp-flags FIN,SYN,RST,PSH,ACK,URG NONE -j DROP
	}

}

basedir="$(readlink -f "$0")"
echo $basedir
config_load firewall
config_foreach add_rules include
