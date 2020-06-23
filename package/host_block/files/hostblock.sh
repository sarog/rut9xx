#!/bin/sh
# Copyright (C) 2017 Teltonika
. /lib/functions.sh

SCRIPT_NAME=$(basename $0)
SERVER_CONF="/tmp/dnsmasq.d/server"
SERVER_SNAP=""
ADDRESS_CONF="/tmp/dnsmasq.d/address"
ADDRESS_SNAP=""
DEFAULT_DNS="8.8.8.8"
DEFAULT_BLOCKIP="255.255.255.255"
SERVER_IP=""


help() {
	echo "Next Generation Host Blocker"
	echo "Usage: $SCRIPT_NAME enable|disable|restart"
}

append_host() {
	local enabled
	local host
	config_get_bool enabled "$1" "enabled"
	if [ $enabled -eq 1 ]; then
		config_get host "$1" "host"
		if [ -n "$host" ]; then
			echo "server=/$host/$SERVER_IP" >> "$SERVER_CONF"
		else
			logger -t "$SCRIPT_NAME" "No host specified"
		fi
	fi
}

enable_hb() {
	local enabled
	local mode
	local icmp_host

	config_load "hostblock"
	config_get_bool enabled "config" "enabled"
	config_get mode "config" "mode"
	config_get icmp_host "config" "icmp_host" "$DEFAULT_DNS"

	if [ $enabled -ne 1 ]; then
		return 1
	fi

	if [ "$mode" = "blacklist" ]; then
		SERVER_IP=""
	elif [ "$mode" = "whitelist" ]; then
		SERVER_IP="$icmp_host"
	else
		logger -t "$SCRIPT_NAME" "No mode specified"
		return 1
	fi

	rm -f $SERVER_CONF
	rm -f $ADDRESS_CONF

	config_foreach append_host "block"

	echo "server=/rms.teltonika.lt/$icmp_host" >> "$SERVER_CONF"

	if [ "$mode" = "whitelist" ]; then
		echo "address=/#/$DEFAULT_BLOCKIP" >> "$ADDRESS_CONF"
	fi

	if [ -e /lib/uci/upload/cbid.hostblock.config.site_blocking_hosts ]; then
		while read -r line || [[ -n "$line" ]]; do
			if [[ -n "$line" ]]; then
				#funkcija skirta nuimti perpildytas eilutes www antrastemis ir windows naujos eilutes simboli
				line=$(echo $line | sed 's/\r$//' | sed 's~http[s]*://[w\.]*~~g' | sed 's~/.*~~' )
				echo "server=/$line/$SERVER_IP" >> "$SERVER_CONF"
			fi
		done < /lib/uci/upload/cbid.hostblock.config.site_blocking_hosts
	fi

	echo "HostBlock enabled"
	return 0
}

disable_hb() {
	rm -f $SERVER_CONF
	rm -f $ADDRESS_CONF
	echo "HostBlock disabled"
}

add_dns_redirect() {
	local cfg
	cfg=$(uci -q add firewall redirect)
	uci -q set firewall.$cfg.enabled='0'
	uci -q set firewall.$cfg.target='DNAT'
	uci -q set firewall.$cfg.src='lan'
	uci -q set firewall.$cfg.dest='lan'
	uci -q set firewall.$cfg.proto='tcp udp'
	uci -q set firewall.$cfg.name='Redirect_DNS'
	uci -q set firewall.$cfg.dest_ip='192.168.1.1'
	uci -q set firewall.$cfg.src_dport='53'
	uci -q set firewall.$cfg.dest_port='53'
}

enable_dns_redirect() {
	local lan_ip
	local rule
	lan_ip=$(uci -q get network.lan.ipaddr)
	if [ -n "$lan_ip" ]; then
		rule=$(uci -q get firewall.REDIR_DNS)
		if [ "$rule" != "redirect" ]; then
			add_dns_redirect
		fi
		uci -q set firewall.REDIR_DNS.enabled=1
		uci -q set firewall.REDIR_DNS.dest_ip="$lan_ip"
		uci -q commit firewall
		reload_firewall
	else
		disable_dns_redirect
	fi
}

disable_dns_redirect() {
	uci -q set firewall.REDIR_DNS.enabled=0
	uci -q commit firewall
	reload_firewall
}

reload_firewall() {
	/etc/init.d/firewall reload >/dev/null 2>&1
}

restart_dnsmasq() {
	/etc/init.d/dnsmasq restart >/dev/null 2>&1
}

config_snapshot() {
	if [ -e "$SERVER_CONF" ]; then
		SERVER_SNAP=$(md5sum "$SERVER_CONF")
	else
		SERVER_SNAP="0"
	fi

	if [ -e "$ADDRESS_CONF" ]; then
		ADDRESS_SNAP=$(md5sum "$ADDRESS_CONF")
	else
		ADDRESS_SNAP="0"
	fi
}

compare_snapshot() {
	local server_snap
	local address_snap
	local ret

	if [ -e "$SERVER_CONF" ]; then
		server_snap=$(md5sum "$SERVER_CONF")
	else
		server_snap="0"
	fi

	if [ -e "$ADDRESS_CONF" ]; then
		address_snap=$(md5sum "$ADDRESS_CONF")
	else
		address_snap="0"
	fi

	if [ "$server_snap" = "$SERVER_SNAP" ] &&
		[ "$address_snap" = "$ADDRESS_SNAP" ]; then
		ret=0
	else
		ret=1
	fi

	return $ret
}

if [ $# -ne 1 ]; then
	help
	exit
fi

case "$1" in
	"enable")
		config_snapshot
		enable_hb
		if [ $? -eq 0 ]; then
			enable_dns_redirect
		fi
	;;
	"disable")
		config_snapshot
		disable_hb
		disable_dns_redirect
	;;
	"restart")
		config_snapshot
		disable_hb
		enable_hb
		if [ $? -eq 0 ]; then
			enable_dns_redirect
		fi
	;;
	*)
		help
		exit
	;;
esac

compare_snapshot
if [ $? -eq 1 ]; then
	echo "Restarting dnsmasq"
	restart_dnsmasq
fi
