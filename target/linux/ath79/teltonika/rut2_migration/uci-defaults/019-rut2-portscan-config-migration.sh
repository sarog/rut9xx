#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

move_option() {
        local option="$1"
        local new_option="$2"
        local section="$3"

        config_get value "$section" "$option"
        [ -n "$value" ] || return 0

        uci_set firewall pscan "$new_option" "$value"
}

move_portscan() {
        move_option enable port_scan "$1"
        move_option seconds seconds "$1"
        move_option hitcount hitcount "$1"
}

move_defending() {
        move_option syn_fin syn_fin "$1"
        move_option syn_rst syn_rst "$1"
        move_option x_max x_max "$1"
        move_option nmap_fin nmap_fin "$1"
        move_option null_flags null_flags "$1"
}

config_load portscan

uci set firewall.pscan=include
uci_set firewall pscan type 'script'
uci_set firewall pscan path '/etc/port-scan-prevention.sh'
config_foreach move_portscan port_scan
config_foreach move_defending defending
uci commit firewall
rm /etc/config/portscan
