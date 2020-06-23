#!/bin/sh

lock /tmp/messaged_lockfilez
	if [ "$1" == "unsolicited_special_sms" ]; then
		sleep 7
	fi
	
	if [ "$1" == "unsolicited_special_email" ]; then
		sleep 5
	fi
	
	/usr/sbin/messaged "$1"
lock -u /tmp/messaged_lockfilez
