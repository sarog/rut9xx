#!/bin/sh
# (C) 2016 Teltonika

number="$1"
text="$2"
date="date +%d -D %s -d $(( $(date +%s) + 86400 ))"

if [ 01 -eq `$date` ]; then
    scheduled_sms_sender.sh $number $text
fi
