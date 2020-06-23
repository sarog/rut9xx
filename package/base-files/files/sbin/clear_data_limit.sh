#! /bin/sh
datalimit=$(uci -q get mdcollectd.config.datalimit)
sim_switch=$(uci -q get mdcollectd.config.sim_switch)
enb_datalimit=0
enb_sim_switch=0
sim=$1

if [ "$sim" != "0" -a "$sim" != "1" ]; then
    echo 1
    exit 1
fi

if  [ "$datalimit" = "1" ]; then
    /etc/init.d/limit_guard stop 
     enb_datalimit="1"
fi

if  [ "$sim_switch" = "1" ]; then
    /etc/init.d/sim_switch stop
    enb_sim_switch="1"
fi

#~ Pasalinam standartines duombazes
/usr/bin/mdcollectdctl -hard_clear$sim
echo 0 > /tmp/limit_total_data
sleep 1

if  [ "$enb_datalimit" = "1" ]; then
     uci -q set network.ppp.enabled=1
     uci -q set network.ppp.overlimit=0
     uci commit
     #ifup ppp
     #~ luci-reload 2>/dev/null 1>/dev/null
    /etc/init.d/limit_guard start 2>/dev/null
fi

if  [ "$enb_sim_switch" = "1" ]; then
    /etc/init.d/sim_switch start
fi

echo 0
exit 0
