#!/bin/sh
. /lib/functions.sh
. /lib/functions/procd.sh

tunnel_enabled="0"
ev_enabled="0"
snmpd_enabled="0"
oc_enabled="0"
lt_enabled="0"
vrrpd_enabled="0"
openvpn_enabled="0"
dropbear_enabled="0"
sc_enabled="0"
en_sv=0
dis_sv=0

fn_exists() {
	type $1 2>/dev/null | grep -q 'is a shell function'
}

en() {
	echo "Enabling $1..."
	/etc/init.d/$1 enabled
	if [ $? -eq 0 ]; then
		echo "$1 already enabled"
		return 1
	fi
	/etc/init.d/$1 enable
	en_sv=$(( en_sv + 1 ))
}

dis() {
	echo "Disabling $1..."
	/etc/init.d/$1 enabled
	if [ $? -ne 0 ]; then
		echo "$1 already disabled"
		return 1
	fi
	/etc/init.d/$1 disable
	dis_sv=$(( dis_sv + 1 ))
}

cli() {
	if [ "$(uci get -q cli.status.enable)" != "0" ]; then
		en shellinabox
	else
		dis shellinabox
	fi
}

network() {
	# Negalim nieko keist
	return 0
}

bridge_arp() {
	# Negalim nieko keist
	return 0
}

firewall() {
	# Negalim nieko keist
	return 0
}

dnsmasq() {
	# Negalim nieko keist
	return 0
}

validate_section_dropbear()
{
	uci_validate_section dropbear dropbear "${1}" \
		'PasswordAuth:bool:1' \
		'enable:bool:1' \
		'Interface:string' \
		'GatewayPorts:bool:0' \
		'RootPasswordAuth:bool:1' \
		'RootLogin:bool:1' \
		'rsakeyfile:file' \
		'dsskeyfile:file' \
		'BannerFile:file' \
		'Port:list(port):22' \
		'SSHKeepAlive:uinteger:300' \
		'IdleTimeout:uinteger:0'
	return $?
}

dropbear_check() {
	local PasswordAuth enable Interface GatewayPorts \
		RootPasswordAuth RootLogin rsakeyfile \
		dsskeyfile BannerFile Port

	validate_section_dropbear "${1}" || return 1
	[ "${enable}" = "0" ] && return 1
	dropbear_enabled="1"
}

dropbear() {
	config_load dropbear
	config_foreach dropbear_check dropbear

	if [ "$dropbear_enabled" = "1" ]; then
		en dropbear	
	else
		dis dropbear
	fi
}

dhcp() {
	return 0
}

qos() {
	# Nieko negalim padaryt
	return 0
}

led() {
	# Nieko negalim keist
	return 0
}

easycwmp() {
	if [ "$(uci get -q easycwmp.@acs[0].enabled)" = "1" ]; then
		en easycwmpd
	else
		dis easycwmpd
	fi
}

ledsman() {
	# Nieko negalim keist
	return 0
}

ntpclient() {
	return 0
}

samba() {
	config_load samba
	if [ "$(uci get -q samba.@samba[0].enable)" = "1" ]; then
		en samba
	else
		dis samba
	fi
}

multiwan() {
	if [ "$(uci get -q multiwan.config.enabled)" -gt 0 ]; then
		en multiwan
	else
		dis multiwan
	fi
}

ping_reboot() {
	if [ "$(uci get -q ping_reboot.ping_reboot.enable)" = "1" ]; then
		en ping_reboot
	else
		dis ping_reboot
	fi
}

el_check() {
	config_get enable "$1" "enable" "0"
	if [ "$enable" = "1" ]; then
		ev_enabled="1"
	fi
}

eventslog_report() {
	config_load eventslog_report
	config_foreach el_check "rule"
	if [ "$ev_enabled" = "1" ]; then
		en eventslog_report
	else
		dis eventslog_report
	fi
}

# socat_check() {
# 	config_get enable "$1" "enable" "0"
# 	if [ "$enable" != "0" ]; then
# 		sc_enabled="1"
# 	fi
# }
#
# socat() {
# 	config_load "socat"
# 	config_foreach socat_check socat
# 	if [ "$sc_enabled" = "1" ]; then
# 		en socat
# 	else
# 		dis socat
# 	fi
# }

auto_update() {
	return 0
}

httpd() {
	return 0
}

reregister() {
	if [ "$(uci get -q reregister.reregister.enabled)" = "1" ]; then
		en reregister
	else
		dis reregister
	fi
}

fstab() {
	return 0
}

uhttpd() {
	# Nieko negalim padaryt
	return 0
}

check_tunnel() {
	local cfg="$1"
	config_get enabled "$cfg" "enabled" "0"
	if [ "$enabled" = "1" ]; then
		tunnel_enabled="1"
	fi
}

gre_tunnel() {
	config_load 'gre_tunnel'
	config_foreach check_tunnel 'gre_tunnel'
	if [ "$tunnel_enabled" = "1" ]; then
		en gre-tunnel
	else
		dis gre-tunnel
	fi
}

racoon() {
	config_load racoon
	local ena
	config_get ena ipsec1 enabled "0"
	if [ "$ena" = "1" ]; then
		en racoon
	else
		dis racoon
	fi
}

sms_utils() {
	# Negalim nieko padaryt
	return 0
}

events_reporting() {
	return 0
}

coovachilli() {
	# Negalim nieko padaryt
	return 0
}

radiusd() {
	local enabled
	config_load "radius"
	config_get enabled "general" "enabled"

	if [ "$enabled" = "1" ]; then
		en radiusd
	else
		dis radiusd
	fi
}

radius() {
	radiusd
}

tcplogger() {
	if [ "$(uci get -q tcplogger.general.enabled)" = "1" ]; then
		en tcplogger
	else
		dis tcplogger
	fi
}

snmpd_check() {
	local cfg="$1"
	config_get enabled "$cfg" enabled
	trap_enabled=`uci -q get snmpd.@trap[0].trap_enabled`
	if [ "$enabled" != "1" ] && [ "$trap_enabled" != "1" ]; then
		return
	fi
	snmpd_enabled="1"
}

snmpd() {
	config_load snmpd

	config_foreach snmpd_check agent
	config_foreach snmpd_check snmpd

	if [ "$snmpd_enabled" = "1" ]; then
		en snmpd
	else
		dis snmpd
	fi
}

check_openvpn() {
	config_get_bool enable  "$1" 'enable'  0
	config_get_bool enabled "$1" 'enabled' 0
	if [ $enable -gt 0 -o $enabled -gt 0 ]; then
		openvpn_enabled="1"
	fi
}

openvpn_vpn() {
	openvpn
}

openvpn() {
	#Del returno kartais nepadaro rc.d nuorodu ir kartais neijungia openvpn, kadangi geriausia kad butu ijungtas panaikinsim dis
	#return 0 # We leave this on

	config_load openvpn
	config_foreach check_openvpn 'openvpn'

	if [ "$openvpn_enabled" = "1" ]; then
		en openvpn
	#else
		#dis openvpn
	fi
}

mdcollectd() {
	if [ "$(uci get -q mdcollectd.config.enabled)" = "0" -a \
	     "$(uci get -q mdcollectd.config.traffic)" = "0" -a \
	     "$(uci get -q mdcollectd.config.datalimit)" = "0" -a \
	     "$(uci get -q mdcollectd.config.sim_switch)" = "0" -a \
	     "$(uci get -q rms_connect_mqtt.rms_connect_mqtt.enable)" = "0" ]; then
		dis mdcollectd
	else
		en mdcollectd
	fi
}

smscollect() {
	if [ "$(uci get -q smscollect.config.enabled)" = "1" ]; then
		en smscollect
	else
		dis smscollect
	fi
}

hostblock() {
	config_load "hostblock"
	config_get_bool enabled "config" "enabled" "0"

	if [ $enabled -eq 1 ]; then
		en hostblock
	else
		dis hostblock
	fi
}

limit_guard() {
	return 0
}

privoxy() {
	config_load "privoxy"
	config_get enabled "privoxy" "enabled" "0"
	if [ "$enabled" = "1" ]; then
		en privoxy
	else
		dis privoxy
	fi
}

sim_switch() {
	if [ "$(uci -q get sim_switch.sim_switch.enabled)" != "0" ]; then
		en sim_switch
	else
		dis sim_switch
	fi
}

gps() {
	gpsd
}

gpsd() {
	if [ "$(uci -q get gps.gps.enabled)" = "1" ]; then
		en gpsd
	else
		dis gpsd
	fi
}

xl2tpd() {
	return 0
}

rs485() {
	if [ "$(uci -q get rs.rs485.enabled)" = "1" ]; then
		en rs485
	else
		dis rs485
	fi
}

rs232() {
	rs
}

rs() {
	if [ "$(uci -q get rs.rs232.enabled)" = "1" ]; then
		en rs232
	else
		dis rs232
	fi
}

periodic_reboot() {
	if [ "$(uci -q get periodic_reboot.periodic_reboot.enable)" = "1" ]; then
		en periodic_reboot
	else
		dis periodic_reboot
	fi
}

check_vrrpd() {
	local enabled
	[ "$1" == "ping" ] && return 1
	config_get_bool enabled "$1" 'enabled' 0
	if [ $enabled -gt 0 ]; then
		vrrpd_enabled="1"
	fi
}

vrrpd() {
	config_load vrrpd
	config_foreach check_vrrpd 'vrrpd'
	if [ "$vrrpd_enabled" = "1" ]; then
		en vrrpd
	else
		dis vrrpd
	fi
}

vrrp_check() {
	if [ "$(uci get -q vrrpd.ping.enabled)" = "1" ]; then
		en vrrp_check
	else
		dis vrrp_check
	fi
}

simpin() {
	dis simpin
}

gsmd_c() {
	# Nieko negalime padaryti
	return 0
}

simcard() {
	# Nieko negalime padaryti
	return 0
}

pptpd() {
	config_load pptpd
	config_get enabled "pptpd" enabled 0
	if [ "$enabled" != "0" ]; then
		en pptpd
	else
		dis pptpd
	fi
}

reregister() {
	if [ "$(uci get -q reregister.reregister.enabled)" = "1" ]; then
		en reregister
	else
		dis reregister
	fi
}

oc_check() {
	config_get enabled $1 enabled "0"
	if [ "$enabled" != "0" ]; then
		oc_enabled="1"
	fi
}

output_control() {
	config_load "output_control"
	config_foreach oc_check 'rule'

	if [ "$oc_enabled" = "1" ]; then
		en output_control
	else
		dis output_control
	fi
}

ddns() {
	return 0
}

lt_check() {
	config_get enabled "$1" "enabled" "0"
	if [ "$enabled" = "1" ]; then
		lt_enabled="1"
	fi
}

logtrigger() {
	config_load logtrigger
	config_foreach lt_check "rule"
	if [ "$lt_enabled" = "1" ]; then
		en logtrigger
	else
		dis logtrigger
	fi
}

ioman() {
	# Negalim nieko keisti
	return 0
}

sms_gateway() {
	if [ "$(uci get -q sms_gateway.pop3.enabled)" = "1" ]; then
		en pop3_ets
	else
		dis pop3_ets
	fi
}

sim_idle_protection() {
	if [ "$(uci -q get sim_idle_protection.sim1.enable)" = "1" -o "$(uci -q get sim_idle_protection.sim2.enabled)" = "1" ]; then
		en sim_idle_protection
	else
		dis sim_idle_protection
	fi
}

smpp_config() {
	smpp_init
}

smpp_init() {
	if [ "$(uci -q get smpp_config.smpp.enabled)" = "1" ]; then
		en smpp_init
	else
		dis smpp_init
	fi
}

dhcprelay() {
	if [ "$(uci -q get dhcp.dhcp_relay.enabled)" = "1" ]; then
		en dhcprelay
	else
		dis dhcprelay
	fi
}

checkall() {
	gps
	rs
	rs485
	multiwan
	ping_reboot
	samba
	sim_switch
	privoxy
	eventslog_report
	racoon
	snmpd
	reregister
	pptpd
	output_control
	smpp_init
	logtrigger
	sim_idle_protection
	vrrpd
	vrrp_check
	simpin
	radius
	openvpn_vpn
	mdcollectd
	smscollect
	hostblock
	limit_guard
	tcplogger
	easycwmp
	gre_tunnel
	periodic_reboot
# 	socat
	cli
	sms_gateway
	reregister
	dhcprelay
	echo "Summary: $en_sv enabled, $dis_sv disabled"
}

if [ $# -eq 0 ]; then
	checkall
else
	# Check the arguments one by one
	for arg in "$@"; do
		if [ "$arg" = "gre-tunnel" ]; then
			gre_tunnel
		else
			fn_exists "$arg" && "$arg"
		fi
	done
fi
