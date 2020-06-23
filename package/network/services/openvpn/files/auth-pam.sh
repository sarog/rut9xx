#!/bin/sh

# $1 - OpenVPN client username/password file location. Username and password are separated by a space.
# $config - OpenVPN ENV var specifying configuration file name

if [ -z $config ]; then
	logger -t "openvpn.$ovpn_inst" "OpenVPN authentication failed"
	exit 1
fi

ovpn_inst=$(echo $config | sed 's/openvpn-//g' | sed 's/.conf//g')
auth_file="/etc/openvpn/auth_$ovpn_inst"

creds=$(cat $1)
cl_usr=$(echo $creds | cut -d' ' -f1)
cl_pass=$(echo $creds | cut -d' ' -f2)

if [ -z "$cl_usr" ] || [ -z "$cl_pass" ] || [ ! -f "$auth_file" ]; then
	logger -t "openvpn.$ovpn_inst" "OpenVPN authentication failed"
	exit 1
fi

while read user_pass; do
	local_usr=$(echo $user_pass | cut -d' ' -f1)
	local_pass=$(echo $user_pass | cut -d' ' -f2)
	if [ "$local_usr" == "$cl_usr" ] && [ "$local_pass" == "$cl_pass" ]; then
		logger -t "openvpn.$ovpn_inst" "OpenVPN authentication successful"
		exit 0
	fi
done < $auth_file

logger -t "openvpn.$ovpn_inst" "OpenVPN authentication failed"
exit 1
