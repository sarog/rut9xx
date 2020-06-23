#!/bin/sh
set -x

SIM=$1
LOCK_FILE=/tmp/sim_idle.lock
NAME=`basename $0`
SIM_CHANGED=0
RELOAD_NETWORK=0

[ "$SIM" != "sim1" -a "$SIM" != "sim2" ] && return

#Tikriname ir nepasileides kitas instance. Jei pasileides, laukiame jo darbo pabaigos
if [ -f $LOCK_FILE ]; then
	pid=`cat $LOCK_FILE`
	
	if  [ -e /proc/$pid ]; then
		i=1
		while [ $i -lt 120 ]
		do
			sleep 5

			if [ ! -e /proc/$pid ]; then
				break
			fi
			i=$((i+1))
		done
		
		#instance pakibes todel kill'inam 
		[ $i -eq 120 ] && kill -9 $pid
		
		
	else
		rm $LOCK_FILE
	fi		
fi

echo $$ > $LOCK_FILE

set +x

. /lib/teltonika-functions.sh

MOBILE_SECTION=`get_wan_section type mobile`
SIM_CHANGED=0
RELOAD_NETWORK=0

set -x
#---------------NETWORK CONF--------------
CONFIG_GET_WAN="uci get -q network.$MOBILE_SECTION"
CONFIG_SET_WAN="uci set -q network.$MOBILE_SECTION"
CONFIG_GET_PPP="uci get -q network.ppp"
CONFIG_SET_PPP="uci set -q network.ppp"
#-----------------------------------------
#-----------HOST TO PING IP---------------
SIM_PING_IP=`uci get -q sim_idle_protection.$SIM.host`
SIM_PING_C=`uci get -q sim_idle_protection.$SIM.count`
SIM_PING_S=`uci get -q sim_idle_protection.$SIM.packet_size`
#-----------------------------------------
#-------------PPP settings----------------
PPP_ENABLED=`$CONFIG_GET_PPP.enabled`
PPP_DISABLED=`$CONFIG_GET_PPP.disabled`
PPP_CHANGED=0
#-----------------------------------------
#------------Mobile WAN settings----------
WAN_ENABLED=`$CONFIG_GET_WAN.enabled`
WAN_DISABLED=`$CONFIG_GET_WAN.disabled`
WAN_CHANGED=0

#Debug
debug(){
	logger -t "$NAME" "$1"
}
#Gaunam aktyvia kortele
get_sim(){
	local curr_sim=`/sbin/gpio.sh get SIM`
	
	[ $curr_sim -eq 1 ] && echo "sim1" || echo "sim2"
}

#Grazina config
set_conf_back(){

	while read line
	do
		uci set $line
	done < /tmp/sim_idle_tmp
	
	rm /tmp/sim_idle_tmp
}

wait_for_connection()
{
	local COUNT=0
	STATE=`gsmctl -j`
	while [ "$STATE" = "disconnected" ] ; do
		STATE=`gsmctl -j`
		COUNT=$((COUNT + 1))
		sleep 5
		if [ $COUNT -eq 10 ] ; then
			break
		fi
	done
}

wait_for_sim()
{
	local SEARCH=0
	local COUNT=0
	local SIM=`gsmctl -z`
	if [ "$SIM" == "inserted" ]; then
		return 1
	else
		if [ "$SIM" == "" ]; then
			while [ "$SIM" = "" ] ; do
				sleep 5
				SIM=`gsmctl -z`
				COUNT=$((COUNT + 1))
			
				if [ $COUNT -eq 10 ] ; then
					return 0
				fi
			done
			
			[ "$SIM" == "inserted" ] && return 1 || return 0
		else
			return 0
		fi
	fi
	
}

#Ijungiame mobile PPP ie WAN configuose
set_mobile(){
	if [ "$PPP_DISABLED" == "1" -o "$PPP_ENABLED" == "0" ]; then
		$CONFIG_SET_PPP.enabled=1
		$CONFIG_SET_PPP.disabled=0
		PPP_CHANGED=1
	fi
	
	if [ "$WAN_DISABLED" == "1" -o "$WAN_ENABLED" == "0" ]; then
		$CONFIG_SET_WAN.enabled=1
		$CONFIG_SET_WAN.disabled=0
		WAN_CHANGED=1
	fi
}

#PPP ir mobile WAN config nustatome i pradine stadija
reset_mobile(){
	if [ $PPP_CHANGED -eq 1 ]; then
		$CONFIG_SET_PPP.enabled=$PPP_ENABLED
		$CONFIG_SET_PPP.disabled=$PPP_DISABLED
	fi
	
	if [ $WAN_CHANGED -eq 1 ]; then
		$CONFIG_SET_WAN.enabled=$WAN_ENABLED
		$CONFIG_SET_WAN.disabled=$WAN_DISABLED
	fi
}

main(){
	local CURRENT_SIM=`get_sim`
	
	#Keiciame sim kortele
	if [ "$SIM" != $CURRENT_SIM ]; then
		debug "Switching to $SIM sim card"
		/usr/sbin/sim_switch change
		CURRENT_SIM=`get_sim`
		SIM_CHANGED=1
		#Laukiame kol startuos modemas
		sleep 20
	fi

	wait_for_sim
	if [ $? -a $? -eq 1 ] ; then
		set_mobile
		[ $PPP_CHANGED -eq 1 -o $WAN_CHANGED -eq 1 -o $SIM_CHANGED -eq 1 ] && /etc/init.d/network reload
		
		#Cia pataisyti
# 		if [ "$GET_BACK_WAN" = 1 ] ; then
# 			sleep 20
# 		else 
# 			sleep 15
# 		fi
		
		wait_for_connection
		local interface=`$CONFIG_GET_PPP.ifname`
		ping $SIM_PING_IP -W 1 -c $SIM_PING_C -s $SIM_PING_S -I $interface > /dev/null 2>&1
		reset_mobile
		
		[ $PPP_CHANGED -eq 1 -o $WAN_CHANGED -eq 1 -o $SIM_CHANGED -eq 1 ] && RELOAD_NETWORK=1
	else
		debug "sim card $CURRENT_SIM not inserted"
	fi

	#Perjungiame i pradine sim kortele
	if [ "$SIM" == "$CURRENT_SIM" -a $SIM_CHANGED -eq 1 ]; then
		debug "Switching back sim card"
		/usr/sbin/sim_switch change default
	fi

	if [ $RELOAD_NETWORK -eq 1 ]; then
		/etc/init.d/network reload
		RELOAD_NETWORK=0
	fi
}

main

[ -f $LOCK_FILE ] && rm $LOCK_FILE

set +x
