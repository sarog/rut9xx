#!/bin/sh
#/etc/racoon/firewall.sh - version 12

. /etc/functions.sh

GetZone() {
  config_get zone "$1" zone vpn
}

GetSA() {
  local remote_subnet
  local local_subnet
  local local_nat
  local remote_lan
  local remote_mask

  #config_get remote_subnet	"$1" remote_subnet
  #config_get local_subnet	"$1" local_subnet
  config_get local_nat		"$1" local_nat ""
  config_get remote_lan		"$1" remote_lan
  config_get remote_mask	"$1" remote_mask
   
  local_ip=`uci get network.lan.ipaddr`
  local_mask=`uci get network.lan.netmask`
  local_ip=`/bin/ipcalc.sh $local_ip $local_mask | grep NETWORK | cut -d= -f2`
  local_mask=`/bin/ipcalc.sh $local_ip $local_mask | grep PREFIX | cut -d= -f2`
  local_subnet=$local_ip"/"$local_mask
  remote_subnet=$remote_lan"/"$remote_mask
  
  

  iptables -A zone_${zone}_ACCEPT -d $remote_subnet -j ACCEPT
  iptables -A zone_${zone}_ACCEPT -s $remote_subnet -j ACCEPT
  iptables -A zone_${zone}_REJECT -d $remote_subnet -j reject
  iptables -A zone_${zone}_REJECT -s $remote_subnet -j reject
  iptables -A zone_${zone}_FORWARD -s $remote_subnet -j zone_${zone}_forward

  if [ "$local_nat" == "" ]; then
    iptables -t nat -A zone_${zone}_nat -d $remote_subnet -j ACCEPT
  else
    iptables -t nat -A zone_${zone}_nat -d $remote_subnet \
             -s $local_subnet -j NETMAP --to $local_nat
    iptables -t nat -A prerouting_${zone} -s $remote_subnet \
             -d $local_nat -j NETMAP --to $local_subnet
  fi
}

GetTunnel() {
  local enabled
  local remote

  config_get_bool enabled "$1" enabled 0
  config_get      remote  "$1" remote
  [[ "$enabled" == "0" ]] && return

  config_list_foreach "$1" sainfo GetSA
}

zone=vpn
config_load racoon
config_foreach GetZone racoon

iptables -F zone_${zone}_ACCEPT
iptables -N zone_${zone}_gateway
iptables -I input -j zone_${zone}
iptables -I input -j zone_${zone}_gateway
iptables -t nat -F zone_${zone}_nat
iptables -t nat -I POSTROUTING 2 -j zone_${zone}_nat
iptables -t nat -I PREROUTING 2 -j zone_${zone}_prerouting

# open IPsec endpoint
iptables -A zone_${zone}_gateway -p esp -j ACCEPT
iptables -A zone_${zone}_gateway -p udp --dport 500 -j ACCEPT
iptables -A zone_${zone}_gateway -p udp --dport 4500 -j ACCEPT

# sort VPN rules to top of forward zones and insert VPN reject marker afterwards

ForwardZones=`iptables -S | awk '/.N.*zone.*_forward/{print $2}' | grep -v ${zone}`
for ForwardZone in $ForwardZones ; do
  echo "iptables -F $ForwardZone" > /tmp/fwrebuild
  iptables -S $ForwardZone | grep zone_${zone}_ACCEPT | \
    grep -v "^-N" | awk '{ print "iptables " $0}' >> /tmp/fwrebuild
  echo "iptables -A $ForwardZone -j zone_${zone}_REJECT" >> /tmp/fwrebuild
  iptables -S $ForwardZone | grep -v zone_${zone}_ACCEPT | \
    grep -v "^-N" | awk '{ print "iptables " $0}' >> /tmp/fwrebuild

  chmod +x /tmp/fwrebuild
  /tmp/fwrebuild
  rm /tmp/fwrebuild
done

# link zone_vpn_forward via zone_vpn_FORWARD
iptables -N zone_${zone}_FORWARD
iptables -I forward -j zone_${zone}_FORWARD

config_foreach GetTunnel tunnel

