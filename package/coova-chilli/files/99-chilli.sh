#!/bin/sh
#Script for old fashioned config compatibility

. /lib/functions.sh

SECTION=
ENABLED=0
DISABLED=1

hotspot_enabled(){
    local section=$1 enabled

    config_get enabled ${section} enabled "0"

    [[ "$enabled" = "1" ]] && {
        ENABLED=1
        DISABLED=0
    }
}

check_hotspot(){
    config_load coovachilli
    config_foreach hotspot_enabled general
}

find_section(){
    local section=$1 option=$2 name=$3 value

    [[ -n "$option" -a -n "$name" ]] || return 0

    config_get value ${section} ${option}
    [[ "X$name" = "X$value"  ]] && SECTION=${section}
}

set_zone(){
    [[ "$(uci -q get firewall.hotspot)" = "zone" ]] && return 0
    config_foreach find_section "zone" "name" "hotspot"

    [[ -n "$SECTION" ]] && uci -q delete firewall.${SECTION}

    logger -t "99-chilli" "Repairing firewall zone"
    uci batch <<-EOF
        set firewall.hotspot=zone
        set firewall.hotspot.name='hotspot'
        set firewall.hotspot.input='REJECT'
        set firewall.hotspot.output='ACCEPT'
        set firewall.hotspot.forward='REJECT'
        set firewall.hotspot.device='tun0 tun1 tun2 tun3'
EOF
}

set_forwarding(){
    SECTION=
    config_foreach find_section "forwarding" "src" "hotspot"
    [[ -n "$SECTION" ]] && return 0

    logger -t "99-chilli" "Repairing firewall forwards "
    uci batch <<-EOF
        add firewall forwarding
        set firewall.@forwarding[-1].dest='wan'
        set firewall.@forwarding[-1].src='hotspot'
EOF
}

set_rule(){
    [[ "$(uci -q get firewall.Hotspot_input)" = "rule" ]] && return 0
    logger -t "99-chilli" "Repairing firewall rules"
    uci batch <<-EOF
        set firewall.Hotspot_input=rule
        set firewall.Hotspot_input.target='ACCEPT'
        set firewall.Hotspot_input.name='Hotspot_input'
        set firewall.Hotspot_input.src='hotspot'
        set firewall.Hotspot_input.dest_port='53 67-68 444 81 1812 1813 3991 3990'
        set firewall.Hotspot_input.enabled=${ENABLED}
EOF
}

set_firewall(){
    config_load firewall

    set_zone
    set_forwarding
    set_rule

    commit firewall
}

set_http(){
    [[ "$(uci -q get uhttpd.hotspot)" = "uhttpd" ]] && return 0

    logger -t "99-chilli" "Repairing uhttpd config"
    uci batch <<-EOF
        set uhttpd.hotspot=uhttpd
        set uhttpd.hotspot.listen_http='0.0.0.0:81'
        set uhttpd.hotspot.listen_https='0.0.0.0:444'
        set uhttpd.hotspot.enablehttp='1'
        set uhttpd.hotspot.home='/www/hotspot'
        set uhttpd.hotspot.rfc1918_filter='1'
        set uhttpd.hotspot.max_requests='3'
        set uhttpd.hotspot.max_connections='100'
        set uhttpd.hotspot.cert='/etc/uhttpd.crt'
        set uhttpd.hotspot.key='/etc/uhttpd.key'
        set uhttpd.hotspot.cgi_prefix='/cgi'
        set uhttpd.hotspot.script_timeout='600'
        set uhttpd.hotspot.network_timeout='30'
        set uhttpd.hotspot.http_keepalive='20'
        set uhttpd.hotspot.tcp_keepalive='1'
        set uhttpd.hotspot.no_dirlists='1'
        set uhttpd.hotspot.ubus_prefix='/ubus'
        set uhttpd.hotspot.disabled=${DISABLED}
        commit uhttpd
EOF
}

check_hotspot
set_firewall
set_http
