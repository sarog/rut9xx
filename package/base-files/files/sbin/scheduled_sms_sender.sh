#!/bin/sh
# (C) 2020 Teltonika

lock /tmp/scheduled_sms_temp

text=$(uci -q get sms_gateway.@msg[0].message | tee /tmp/.smstext)
is_b64=$(uci -q get sms_gateway.@msg[0].is_b64)
number="$1"

if [ ${is_b64:-0} -eq 1 ]; then
	response=$(gsmctl -Sb "$number" 2>&1)
else
	response=$(gsmctl -Ss "$number $text" 2>&1)
fi

ret=$?
if [ "$response" != "OK" ] || [[ $ret != 0 ]]; then
	logger -t "$0" "gsmctl return code $ret: \"$response\""
fi

lock -u /tmp/scheduled_sms_temp
