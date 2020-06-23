#!/bin/sh

statistics condown

#if mode smsotp, remove tmp user_name
if [ ! -z $J_T5_section ]; then
  hotspot_id=`uci get wireless.$J_T5_section.hotspotid`

  if [ ! -z $hotspot_id ]; then
    mode=`uci get coovachilli.$hotspot_id.mode`

    if [ "$mode" = "sms" -a ! -z $USER_NAME -a ! -z $DHCPIF ]; then
      sed -i "/$USER_NAME/d" /etc/chilli/$DHCPIF/smsusers
    fi
  fi
fi
