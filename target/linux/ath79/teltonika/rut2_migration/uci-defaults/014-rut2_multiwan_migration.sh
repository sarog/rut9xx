#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

insert_interface() {
	local interface="$1"
	local enabled="$2"
	local health_interval priority timeout health_fail_retries
	local health_recovery_retries metric
	local netw_intf

	#Check if network interface exist
	netw_intf=$(uci_get network "$interface")
	[ -z "$netw_intf" ] && return

	config_get health_interval "$interface" health_interval 3
	config_get priority "$interface" priority 1
	config_get timeout "$section" timeout 1
	config_get health_fail_retries "$section" health_fail_retries 3
	config_get health_recovery_retries "$section" health_recovery_retries 3
	config_get icmp_hosts "$section" icmp_hosts '8.8.8.8'

	#~ we need to invert priority, it should not exceed 100
	metric="$((101 - priority))"
	[ "$metric" -le 0 ] && metric=1

	uci_add_list mwan3 mwan_default use_member "${interface}_member_mwan"
	uci_add_list mwan3 balance_default use_member "${interface}_member_balance"

	uci_add mwan3 interface "$interface"
	uci_set mwan3 "$interface" enabled "$enabled"
	uci_set mwan3 "$interface" family ipv4
	uci_set mwan3 "$interface" interval "$health_interval"

	uci_add mwan3 member "${interface}_member_mwan"
	uci_set mwan3 "${interface}_member_mwan" interface "$interface"
	uci_set mwan3 "${interface}_member_mwan" metric "$metric"


	uci_add mwan3 member "${interface}_member_balance"
	uci_set mwan3 "${interface}_member_balance" interface "$interface"
	uci_set mwan3 "${interface}_member_balance" weight "1"

	uci_add mwan3 condition
	uci_set mwan3 "$CONFIG_SECTION" condition
	uci_set mwan3 "$CONFIG_SECTION" interface "$interface"
	uci_set mwan3 "$CONFIG_SECTION" track_method ping
	uci_set mwan3 "$CONFIG_SECTION" reliability 1
	uci_set mwan3 "$CONFIG_SECTION" count 1
	uci_set mwan3 "$CONFIG_SECTION" timeout "$timeout"
	uci_set mwan3 "$CONFIG_SECTION" down "$health_fail_retries"
	uci_set mwan3 "$CONFIG_SECTION" up "$health_recovery_retries"
	for host in $icmp_hosts; do
		uci_add_list mwan3 "$CONFIG_SECTION" track_ip "$host"
	done
}

update_memb() {
	local interface weight netw_intf

	config_get interface "$1" interface

	#Check if network interface exist
	netw_intf=$(uci_get network "$interface")
	[ -z "$netw_intf" ] && return

	config_get weight "$1" weight 1
	uci_set mwan3 "${interface}_member_balance"
	uci_set mwan3 "${interface}_member_balance" weight "$weight"
}

#prepare empty config
: > /etc/config/mwan3

uci_add mwan3 globals globals
uci_set mwan3 globals mmx_mask '0x3F00'
uci_set mwan3 globals rtmon_interval 5

uci_add mwan3 rule default_rule
uci_set mwan3 default_rule dest_ip '0.0.0.0/0'
uci_set mwan3 default_rule use_policy mwan_default

uci_add mwan3 policy mwan_default
uci_add mwan3 policy balance_default

enabled="$(uci_get multiwan config enabled 0)"
[ "$enabled" = 0 ] && {
	#check if load balancing enabled and set policy
	enabled="$(uci_get load_balancing general enabled 0)"
	[ "$enabled" = 1 ] && uci_set mwan3 default_rule use_policy balance_default
}
#prepare multiwan
config_load multiwan
config_foreach insert_interface interface "$enabled"
insert_interface mob1s2a1 "$enabled"

#prepare load balancing in separate env
(
	config_load load_balancing
	config_foreach update_memb member
)

uci_commit mwan3

rm /etc/config/multiwan
rm /etc/config/load_balancing
