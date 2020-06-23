#!/bin/sh
. /lib/functions.sh

CONFIG_FILE="/etc/config/kmod_man"

kmod_toggle() {
	config_get enabled $1 "enabled" "0"
	config_get path $1 "path" ""

	if [ -n "$path" ]; then
		if [ "$enabled" = "0" ] && [ -e "$path" ]; then
			rm -f "$path"
			cmds=$(sed -e 's/^/rmmod /' /rom/"$path" | sed '1!G;h;$!d')
			eval "$cmds"
		elif [ "$enabled" = "1" ] && [ ! -e "$path" ]; then
			cp "/rom""$path" "$path"
			cmds=$(sed -e 's/^/insmod /' /rom/"$path")
			eval "$cmds"
		fi
	fi
}

kmod_init() {
	if [ -e "/rom""$CONFIG_FILE" ] && [ ! -e "$CONFIG_FILE" ]; then
		cp "/rom""$CONFIG_FILE" "$CONFIG_FILE"
	elif [ ! -e "/rom""$CONFIG_FILE" ]; then
		exit 1
	fi

	config_load "kmod_man"
	config_foreach kmod_toggle "module"
}

kmod_init
