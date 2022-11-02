#!/bin/sh

network_hook() {
	local log_file="${PACK_DIR}network.log"

	troubleshoot_init_log "ACTIVE CONNECTIONS" "$log_file"
	troubleshoot_add_log "$(netstat -tupan 2>/dev/null)" "$log_file"

	troubleshoot_init_log "IP RULES" "$log_file"
	troubleshoot_add_log "$(ip rule)" "$log_file"
	troubleshoot_init_log "IP ROUTES" "$log_file"
	troubleshoot_add_log "$(ip route show table all)" "$log_file"

	troubleshoot_init_log "IPSEC STATUS" "$log_file"
	troubleshoot_add_log_ext "ipsec" "statusall" "$log_file"

	troubleshoot_init_log "MULTIWAN STATUS" "$log_file"
	troubleshoot_add_log_ext "mwan3" "status" "$log_file"
}

firewall_hook() {
	local log_file="${PACK_DIR}firewall.log"

	[ -n "$(which iptables)" ] && {
		troubleshoot_init_log "IPtables FILTER" "$log_file"
		troubleshoot_add_log_ext "ip" "tun" "$log_file"
		troubleshoot_add_log "$(iptables -L -nv)" "$log_file"

		troubleshoot_init_log "IPtables NAT" "$log_file" "$log_file"
		troubleshoot_add_log "$(iptables -t nat -L -nv)" "$log_file"

		troubleshoot_init_log "IPtables MANGLE" "$log_file" "$log_file"
		troubleshoot_add_log "$(iptables -t mangle -L -nv)" "$log_file"

		troubleshoot_add_log "$(iptables-save)" "$log_file"
	}

	[ -n "$(which ebtables)"  ] && {
		troubleshoot_init_log "EBtables FILTER" "$log_file"
		troubleshoot_add_log_ext "ip" "tun" "$log_file"
		troubleshoot_add_log "$(ebtables -t filter -L --Lc)" "$log_file"

		troubleshoot_init_log "EBtables NAT" "$log_file"
		troubleshoot_add_log "$(ebtables -t nat -L --Lc)" "$log_file"

		troubleshoot_init_log "EBtables BROUTE" "$log_file"
		troubleshoot_add_log "$(ebtables -t broute -L --Lc)" "$log_file"
	}
}

interfaces_hook() {
	local log_file="${PACK_DIR}network.log"
	local iflist="`ls /sys/class/net/`"
	local dhcp_list_data

	troubleshoot_init_log "Interfaces" "$log_file"
	for iface in $iflist ; do
		ifconfig $iface >> "$log_file" 2>/dev/null
		ip a sh dev $iface >> "$log_file" 2>/dev/null
		echo -e "\n" >> "$log_file" 2>/dev/null
	done

	troubleshoot_init_log "Tunnels" "$log_file"
	troubleshoot_add_log "$(ip tun)" "$log_file"
	troubleshoot_init_log "Bridges" "$log_file"
	troubleshoot_add_log "$(brctl show)" "$log_file"
	troubleshoot_init_log "Routing table" "$log_file"
	troubleshoot_add_log "$(route -n -e)" "$log_file"

	troubleshoot_init_log "DHCP leases" "$log_file"
	dhcp_list_data=$(cat /tmp/dhcp.leases)
	if [ -f /tmp/dhcp.leases ] && [ -n "$dhcp_list_data" ]; then
		troubleshoot_add_log "$dhcp_list_data" "$log_file"
	else
 		troubleshoot_add_log "no DHCP leases.." "$log_file"
	fi

	troubleshoot_init_log "ARP Data" "$log_file"
	troubleshoot_add_log "$(cat /proc/net/arp)" "$log_file"
}

troubleshoot_hook_init interfaces_hook
troubleshoot_hook_init network_hook
troubleshoot_hook_init firewall_hook
