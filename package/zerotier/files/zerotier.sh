#!/bin/sh

cfg="$1"
CONFIG_GET="uci get zerotier.$cfg"
CONFIG_SET="uci set zerotier.$cfg"

ARGS=""

rm -rf /var/lib/zerotier-one/networks.d/
rm -rf /tmp/ZTlog.txt
mkdir -p /var/lib/zerotier-one/networks.d/

if [ -z "$(uci show firewall | grep zero_zone)" ]; then
    uci set firewall.zero_zone=zone
    uci set firewall.zero_zone.name='zero'
    uci set firewall.zero_zone.input='ACCEPT'
    uci set firewall.zero_zone.output='ACCEPT'
    uci set firewall.zero_zone.forward='REJECT'
    uci set firewall.zero_zone.network='zero'
    uci set firewall.zero_zone.device='zt+'
fi
if [ -z "$(uci show firewall | grep ".src='zero'")" ]; then
    tmps="$(uci add firewall forwarding)"
    echo $tmp
    uci set firewall.$tmps.src='zero'
    uci set firewall.$tmps.dest='lan'
fi

if [ -z "$(uci show firewall | grep ".dest='zero'")" ]; then
    tmps="$(uci add firewall forwarding)"
    uci set firewall.$tmps.dest='zero'
    uci set firewall.$tmps.src='lan'
fi

enabled=`$CONFIG_GET.enabled` 2>> /tmp/ZTlog.txt
if [ "$enabled" != 0 ]; then

    port=`$CONFIG_GET.port`  2>> /tmp/ZTlog.txt
    secret=`$CONFIG_GET.secret`  2>> /tmp/ZTlog.txt
    vpnenabled=`$CONFIG_GET.vpnenabled`  2>> /tmp/ZTlog.txt
    mode=`$CONFIG_GET.mode` 2>> /tmp/ZTlog.txt
    selectedNetwork=`$CONFIG_GET.selectedNetwork`  2>> /tmp/ZTlog.txt
    join=`$CONFIG_GET.join`  2>> /tmp/ZTlog.txt

    if [ -n "$port" ]; then
        ARGS="$ARGS -p$port"
    fi

    if [ "$secret" = "generate" ]; then
        #echo "Generate secret - please wait..."
        tmp="/tmp/zt.zerotier.secret"
        zerotier-idtool generate "$tmp" > /dev/null
        secret="$(cat $tmp)"
        rm "$tmp"
        address="$(echo $secret | cut -d':' -f1)"

        $CONFIG_SET.secret="$secret"  2>> /tmp/ZTlog.txt
        $CONFIG_SET.address="$address"  2>> /tmp/ZTlog.txt
        uci commit zerotier
    fi

    if [ -z "$address" ] && [ -n "$secret" ]; then
        address="$(echo $secret | cut -d':' -f1)"

        $CONFIG_SET.address="$address" 2>> /tmp/ZTlog.txt
        uci commit zerotier
    fi

    if [ -n "$secret" ]; then
        echo "$secret" > /var/lib/zerotier-one/identity.secret
        #make sure there is not previous identity.public
        rm -f /var/lib/zerotier-one/identity.public
    fi

    zerotier-one $ARGS &

    while [ ! -f /var/lib/zerotier-one/zerotier-one.port ]; do sleep 2; done

    for f in $join
    do
        touch /tmp/lib/zerotier-one/networks.d/$f.conf
        zerotier-one -q join $f >> /tmp/ZTlog.txt
        zerotier-one -q set $f allowDefault=0 >> /tmp/ZTlog.txt
    done

    if [ $vpnenabled -gt 0 ] && [ "$mode" = "server" ]; then
        uci set firewall.zero_zone.forward=ACCEPT
        uci commit firewall
        sysctl -w "net.ipv4.ip_forward=1" >> /tmp/ZTlog.txt
        echo "Server enabled" >> /tmp/ZTlog.txt
    else
        uci set firewall.zero_zone.forward=REJECT
        uci commit firewall
    fi

    if [ $vpnenabled -gt 0 ] && [ "$mode" = "client" ]; then
        sysctl -w "net.ipv4.conf.all.rp_filter=2" >> /tmp/ZTlog.txt
        sysctl -p >> /tmp/ZTlog.txt

        zerotier-one -q set $selectedNetwork allowDefault=1 >> /tmp/ZTlog.txt
    else
        sysctl -w "net.ipv4.conf.all.rp_filter=1" >> /tmp/ZTlog.txt
        sysctl -p >> /tmp/ZTlog.txt
    fi
else
    kill $(ps | grep '[z]erotier-one' | awk '{print $1}')
fi
