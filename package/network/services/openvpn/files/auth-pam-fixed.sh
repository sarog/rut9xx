#!/bin/sh

# $1 - OpenVPN client username/password file location. Username and password are separated by a space.
# $config - OpenVPN ENV var specifying configuration file name.

[ -z "$config" ] && {
	logger -t "openvpn.$ovpn_inst" "OpenVPN authentication failed, missing config var"
	exit 1
}

ovpn_inst="${config##*openvpn-}"
ovpn_inst="${ovpn_inst%.conf}"
auth_file="/etc/openvpn/auth_${ovpn_inst}"

#Convert from windows to unix line endings
sed -i "s/\r//g" "$auth_file"
#Apply empty line to file ending
[ -n "$(tail -c1 "$auth_file")" ] && echo >> "$auth_file"

cl_usr=$(sed -n '1p' "$1")
cl_pass=$(sed -n '2p' "$1")

[ -z "$cl_usr" ] || [ -z "$cl_pass" ] || [ ! -f "$auth_file" ] && {
	logger -t "openvpn.$ovpn_inst" "OpenVPN authentication failed, missing username or password"
	exit 1
}

while read user pass; do
	[ "$user" = "$cl_usr" ] && [ "$pass" = "$cl_pass" ] && {
		logger -t "openvpn.$ovpn_inst" "OpenVPN authentication successful"
		exit 0
	}
done < "$auth_file"

logger -t "openvpn.$ovpn_inst" "OpenVPN authentication failed, username or password not found"
exit 1
