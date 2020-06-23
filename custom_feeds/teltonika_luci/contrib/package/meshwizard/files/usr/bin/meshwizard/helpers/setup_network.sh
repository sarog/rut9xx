# setup entry in /etc/config/network for a interface
# Argument $1: network interface
 
net="$1"
. /etc/functions.sh
. $dir/functions.sh

# Setup a (new) interface section for $net

ipaddr=$(uci get meshwizard.netconfig.$net\_ip4addr)
[ -z "$ipaddr" ] && msg_missing_value meshwizard $net\_ip4addr

netmask=$(uci -q get meshwizard.netconfig.$net\_netmask)
[ -z "$netmask" ] && netmask="$interface_netmask"
[ -z "$netmask" ] && netmask="255.255.0.0"

uci set network.$netrenamed="interface"
set_defaults "interface_" network.$netrenamed

uci batch << EOF
	set network.$netrenamed.proto="static"
	set network.$netrenamed.ipaddr="$ipaddr"
	set network.$netrenamed.netmask="$netmask"
EOF

uci_commitverbose "Setup interface $netrenamed" network

# setup dhcp alias/interface

net_dhcp=$(uci -q get meshwizard.netconfig.${net}_dhcp)
if [ "$net_dhcp" == 1 ]; then

	# Load meshwizard_settings
	dhcprange="$(uci -q get meshwizard.netconfig.${net}_dhcprange)"
	interface_ip="$(uci -q get meshwizard.netconfig.${net}_ip4addr)"
	vap=$(uci -q get meshwizard.netconfig.${net}_vap)

	# Rename config
	handle_dhcpalias() {
			config_get interface "$1" interface
			if [ "$interface" == "$netrenamed" ]; then
				if [ -z "${1/cfg[0-9a-fA-F]*/}" ]; then
					section_rename network $1 ${netrenamed}dhcp
				fi
			fi
	}
	config_load network
	config_foreach handle_dhcpalias alias

	# Get IP/netmask and start-ip for $net dhcp
	# If no dhcprange is given in /etc/config/meshwizard we autogenerate one

	if [ -z "$dhcprange" ]; then
		dhcprange="$($dir/helpers/gen_dhcp_ip.sh $interface_ip)/24"
		uci set meshwizard.netconfig.${net}_dhcprange="$dhcprange"
	fi
	eval $(sh $dir/helpers/ipcalc-cidr.sh $dhcprange 1 0)

	# setup wifi-dhcp interface or alias

	# Setup alias for $net

	if [ "$vap" == 1 ]; then
		uci set network.${netrenamed}dhcp=interface
	else
		uci set network.${netrenamed}dhcp=alias
		uci set network.${netrenamed}dhcp.interface="$netrenamed"
	fi

	uci batch <<- EOF
		set network.${netrenamed}dhcp.proto=static
		set network.${netrenamed}dhcp.ipaddr="$START"
		set network.${netrenamed}dhcp.netmask="$NETMASK"
	EOF
	uci_commitverbose  "Setup interface for ${netrenamed}dhcp" network

fi
