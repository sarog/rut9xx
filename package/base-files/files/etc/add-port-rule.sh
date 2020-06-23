#! /bin/sh

enable=`uci get -q portscan.@port_scan[0].enable`
if [ "$enable" == "1" ]; then
        seconds=`uci get portscan.@port_scan[0].seconds`
        hitcount=`uci get portscan.@port_scan[0].hitcount`

        iptables -N zone_port_scan

        iptables -A zone_port_scan -p tcp -m state --state NEW -m recent --set
        iptables -A zone_port_scan -p tcp -m state --state NEW -m recent --update --seconds $seconds --hitcount $hitcount -j DROP

        iptables -t filter -I zone_wan_input -j zone_port_scan
        iptables -t filter -I zone_wan_forward -j zone_port_scan

fi

syn_fin=`uci get -q portscan.@defending[0].syn_fin`
if [ "$syn_fin" == "1" ]; then
        iptables -t raw -I PREROUTING -p tcp --tcp-flags FIN,SYN FIN,SYN -j DROP
else
        iptables -t raw -D PREROUTING -p tcp --tcp-flags FIN,SYN FIN,SYN -j DROP
fi

syn_rst=`uci get -q portscan.@defending[0].syn_rst`
if [ "$syn_rst" == "1" ]; then
        iptables -t raw -I PREROUTING -p tcp --tcp-flags SYN,RST SYN,RST -j DROP
else
        iptables -t raw -D PREROUTING -p tcp --tcp-flags SYN,RST SYN,RST -j DROP
fi

x_max=`uci get -q portscan.@defending[0].x_max`
if [ "$x_max" == "1" ]; then
        iptables -t raw -I PREROUTING -p tcp --tcp-flags FIN,SYN,RST,PSH,ACK,URG FIN,PSH,URG -j DROP
else
        iptables -t raw -D PREROUTING -p tcp --tcp-flags FIN,SYN,RST,PSH,ACK,URG FIN,PSH,URG -j DROP
fi

nmap_fin=`uci get -q portscan.@defending[0].nmap_fin`
if [ "$nmap_fin" == "1" ]; then
        iptables -t raw -I PREROUTING -p tcp --tcp-flags FIN,SYN,RST,PSH,ACK,URG FIN -j DROP
else
        iptables -t raw -D PREROUTING -p tcp --tcp-flags FIN,SYN,RST,PSH,ACK,URG FIN -j DROP
fi

null_flags=`uci get -q portscan.@defending[0].null_flags`
if [ "$null_flags" == "1" ]; then
        iptables -t raw -I PREROUTING -p tcp --tcp-flags FIN,SYN,RST,PSH,ACK,URG NONE -j DROP
else
        iptables -t raw -D PREROUTING -p tcp --tcp-flags FIN,SYN,RST,PSH,ACK,URG NONE -j DROP
fi
