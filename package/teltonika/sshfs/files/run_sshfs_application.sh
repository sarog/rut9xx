#!/bin/sh
# Copyright (C) 2021 Teltonika

. /lib/functions.sh

PROG="/usr/bin/sshfs"
DEF_MOUNT_POINT="/sshmount"

check_variable() {
	if [ "$1" == "" ]; then
		logger -s -t "SSHFS" "$2 not set!"
		exit 0
	fi
}

config_load sshfs
config_get enabled "sshfs" enabled 0

if [ $enabled -ne 1 ]; then
	logger -t "SSHFS" "Service not enabled."
	return 0
fi

config_get hostname "sshfs" hostname ""
check_variable "$hostname" hostname

config_get username "sshfs" username ""
check_variable "$username" username

config_get port "sshfs" port ""

config_get password "sshfs" password ""
check_variable "$password" password

config_get mount_path "sshfs" mount_path ""
check_variable "$mount_path" mount_path

config_get mount_point "sshfs" mount_point "$DEF_MOUNT_POINT"

mkdir -p "$mount_point"
echo "$password" | $PROG ${port:+-p $port} -o ssh_command='ssh -y' "$username@$hostname:$mount_path" $mount_point -o password_stdin -o allow_other

