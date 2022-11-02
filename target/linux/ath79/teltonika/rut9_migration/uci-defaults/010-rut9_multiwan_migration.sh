#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

insert_interface() {
	local interface
	local enabled="$2"
	local health_interval priority timeout health_fail_retries
	local health_recovery_retries metric
	local netw_intf

	#Check if network interface exist
	netw_intf=$(uci_get network "$1")
	[ -z "$netw_intf" ] && return

	local ifname="$(uci get network.$1.ifname)"
                                                                                               
        case "$ifname" in
                wwan0 | \
                3g-ppp)
			interface=mob1s1a1
                        ;;                                                                        
                wlan0)
			interface=wwan0
                        ;;                                                                        
                eth1)
			interface=wan                                                           
                        ;;                                                                        
                esac

	config_get health_interval "$1" health_interval 3
	config_get priority "$1" priority 1
	config_get timeout "$1" timeout 1
	config_get health_fail_retries "$1" health_fail_retries 3
	config_get health_recovery_retries "$1" health_recovery_retries 3
	config_get icmp_hosts "$1" icmp_hosts '8.8.8.8'

	#~ we need to invert priority, it should not exceed 100
	metric="$((101 - priority))"
	[ "$metric" -le 0 ] && metric=1

	uci_add_list mwan3 mwan_default use_member "${interface}_member_mwan"

	uci_add mwan3 interface "$interface"
	uci_set mwan3 "$interface" enabled "$enabled"
	uci_set mwan3 "$interface" family ipv4
	uci_set mwan3 "$interface" interval "$health_interval"

	uci_add mwan3 member "${interface}_member_mwan"
	uci_set mwan3 "${interface}_member_mwan" interface "$interface"
	uci_set mwan3 "${interface}_member_mwan" metric "$metric"

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

	[ "$interface" = "mob1s1a1" ] || continue

        #~ we need to invert priority, it should not exceed 100
        metric="$((101 - priority))"
        [ "$metric" -le 0 ] && metric=1

        uci_add_list mwan3 mwan_default use_member "mob1s2a1_member_mwan"

        uci_add mwan3 interface "mob1s2a1"
        uci_set mwan3 "mob1s2a1" enabled "$enabled"
        uci_set mwan3 "mob1s2a1" family ipv4
        uci_set mwan3 "mob1s2a1" interval "$health_interval"

        uci_add mwan3 member "mob1s2a1_member_mwan"
        uci_set mwan3 "mob1s2a1_member_mwan" interface "mob1s2a1"
        uci_set mwan3 "mob1s2a1_member_mwan" metric "$metric"

        uci_add mwan3 condition
        uci_set mwan3 "$CONFIG_SECTION" condition
        uci_set mwan3 "$CONFIG_SECTION" interface "mob1s2a1"
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


move_option() {
	local section="$1"
	local option="$2"
	local new_option="$3"
	local new_section="$4"

	config_get value "$section" "$option"
	[ -n "$value" ] || return 0

	[ "$option" = "use_policy" ] && value="balance_${value}"

	uci_set mwan3 "$new_section" "$new_option" "$value"
}

find_wan() {
	section="$1"
	weight="$3"
	policy="$4"
	[ -z "$(echo "$2" | grep -E '[a-z]+[0-9]+_+[a-z]+[0-9]')" ] || set 2 "default_${2%%_*}"

	[ "$section" = "${2##*_}" ] || continue
	config_get ifname "$section" ifname

	case "$ifname" in
                wwan0 | \
                3g-ppp)
			uci_add mwan3 member "${2%%_*}_mob1s1a1_member_balance"
			uci_set mwan3 "${2%%_*}_mob1s1a1_member_balance" ifname mob1s1a1
			uci_set mwan3 "${2%%_*}_mob1s1a1_member_balance" weight "$weight"
			uci add_list mwan3."$policy".use_member="${2%%_*}_mob1s1a1_member_balance"

			uci_add mwan3 member "${2%%_*}_mob1s2a1_member_balance"
                        uci_set mwan3 "${2%%_*}_mob1s2a1_member_balance" ifname mob1s2a1
                        uci_set mwan3 "${2%%_*}_mob1s2a1_member_balance" weight "$weight"
                        uci add_list mwan3."$policy".use_member="${2%%_*}_mob1s2a1_member_balance"
                        ;;                                                     
                wlan0)                                                         
			uci_add mwan3 member "${2%%_*}_wwan_member_balance"
			uci_set mwan3 "${2%%_*}_wwan_member_balance" ifname wwan
                        uci_set mwan3 "${2%%_*}_wwan_member_balance" weight "$weight"

			uci add_list mwan3."$policy".use_member="${2%%_*}_wwan_member_balance"
                        ;;                                
                eth1)                                          
			uci_add mwan3 member "${2%%_*}_wan_member_balance"
			uci_set mwan3 "${2%%_*}_wan_member_balance" ifname wan        
                        uci_set mwan3 "${2%%_*}_wan_member_balance" weight "$weight"

			uci add_list mwan3."$policy".use_member="${2%%_*}_wan_member_balance"
                        ;;
		esac                                
}

move_list() {
	local value="$1"
        local section="$2"

	config_get weight "$1" weight

	config_load network
	config_foreach find_wan interface "$value" "$weight" "$section"
	config_load load_balancing
}

move_rules() {
	[ "$1" = "default_rule" ] && continue

	local src_port dest_port src_ip dest_ip sticky timeout proto use_policy
	config_get src_ip "$1" src_ip 0
	config_get dest_ip "$1" dest_ip 0

	uci_add mwan3 rule "$1"
	move_option "$1" src_port src_port "$1"
	move_option "$1" dest_port dest_port "$1"
        move_option "$1" sticky sticky "$1"
        move_option "$1" timeout timeout "$1"
        move_option "$1" proto proto "$1"
	move_option "$1" use_policy use_policy "$1"

	[ "$src_ip" -eq 0 ] || uci add_list mwan3."$1".src_ip="$src_ip"
	[ "$dest_ip" -eq 0 ] || uci add_list mwan3."$1".dest_ip="$dest_ip"
}

move_policy() {
	if [ "$1" = "balanced" ]; then
		config_list_foreach "$1" use_member move_list "balance_default"
	else
		uci_add mwan3 policy "balance_${1}"
		config_list_foreach "$1" use_member move_list "balance_${1}"
	fi
}

chng_loaddbal_memb() {
        fix_memb() {
                [ "$2" = "$(config_get "$1" interface)" ] && \
                uci_set load_balancing "$1" interface "$3"
        }

        (
                config_load load_balancing
                config_foreach fix_memb member "$1" "$2"
                uci_commit load_balancing
        )
}

move_interface() {
        case "$ifname" in
                wwan0 | \
                3g-ppp)
			chng_loaddbal_memb "$section" mob1s1a1
			chng_loaddbal_memb "$section" mob1s2a1
			;;
                wlan0)
			chng_loaddbal_memb "$section" wwan                                                        
                        ;;                                                              
                eth1)
			chng_loaddbal_memb "$section" wan
                        ;;                                                         
                esac                                                               
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

#prepare load balancing in separate env
(
	config_load network                                                                               
	config_foreach move_interface interface
	config_load load_balancing
	config_foreach move_rules rule
	config_foreach move_policy policy
)

uci_commit mwan3

rm /etc/config/multiwan
rm /etc/config/load_balancing
