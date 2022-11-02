#!/bin/sh

. /lib/functions.sh

find_action() {
	local sec=$1 name=$2

	config_get action "$sec" action
	[ -n "$action" -a "$action" == "$name" ] && eval "action_$name=1"
}

config_load sms_utils
config_foreach find_action rule userdefaults

[ -z "$action_userdefaults" ] && {
	uci -q batch <<-EOF
        add sms_utils rule
        set sms_utils.@rule[-1].action='userdefaults'
		set sms_utils.@rule[-1].enabled='1'
		set sms_utils.@rule[-1].smstext='userdefaults'
		set sms_utils.@rule[-1].authorization='password'
		set sms_utils.@rule[-1].allowed_phone='all'
		set sms_utils.@rule[-1].to_other_phone='0'
        commit sms_utils
EOF
}
