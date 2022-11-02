#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

fix_openvpn() {
	local sec=$1
	local name=${sec#*_}

	config_get _role "$sec" _role
	config_get _auth "$sec" _auth
	[ -n "$_role" -a -n "$name" ] || return 0

	uci_set "openvpn" "$sec" "_name" "$name"
	uci_set "openvpn" "$sec" "persist_tun" "1"
	uci_set "openvpn" "$sec" "type" "$_role"

	local ifconfig
	local route
	local server

	config_get ifconfig "$sec" ifconfig
	config_get route "$sec" route
	config_get server "$sec" server

	[ -n "$ifconfig" ] && {
		uci_set "openvpn" "$sec" "local_ip" ${ifconfig%% *}
		uci_set "openvpn" "$sec" "remote_ip" ${ifconfig##* }
		uci delete "openvpn"."$sec"."ifconfig"
	}

	[ -n "$route" ] && {
		uci_set "openvpn" "$sec" "network_ip" ${route%% *}
		uci_set "openvpn" "$sec" "network_mask" ${route##* }
		uci delete "openvpn"."$sec"."route"
	}

	[ -n "$server" ] && {
		uci_set "openvpn" "$sec" "server_ip" ${server%% *}
		uci_set "openvpn" "$sec" "server_netmask" ${server##* }
		uci delete "openvpn"."$sec"."server"
	}

	[ "$_auth" = "tls" -o "$_auth" = "tls/pass" ] && {
		uci_set "openvpn" "$sec" "tls_${_role}" "1"
	}

	mkdir -p "/etc/vuci-uploads"
	local ca cert key dh tls_auth tls_crypt crl_verify auth_key_direction secret userpass
	local ca_dir cert_dir key_dir dh_dir tls_auth_dir tls_crypt_dir crl_verify_dir secret_dir userpass_dir
	config_get ca "$sec" ca
	config_get cert "$sec" cert
	config_get key "$sec" key
	config_get dh "$sec" dh
	config_get tls_auth "$sec" tls_auth
	config_get tls_crypt "$sec" tls_crypt
	config_get crl_verify "$sec" crl_verify
	config_get auth_key_direction "$sec" auth_key_direction
	config_get secret "$sec" secret
	config_get userpass "$sec" userpass
		
	ca_dir="/etc/vuci-uploads/${ca##*/}"; ca_dir="${ca_dir/.server_/.}"; ca_dir="${ca_dir/.client_/.}"
	cert_dir="/etc/vuci-uploads/${cert##*/}"; cert_dir="${cert_dir/.server_/.}"; cert_dir="${cert_dir/.client_/.}"
	key_dir="/etc/vuci-uploads/${key##*/}"; key_dir="${key_dir/.server_/.}"; key_dir="${key_dir/.client_/.}"
	dh_dir="/etc/vuci-uploads/${dh##*/}"; dh_dir="${dh_dir/.server_/.}"; dh_dir="${dh_dir/.client_/.}"
	[ "$tls_auth" != "${tls_auth##* }" ] && {
		[ -n "$auth_key_direction" ] && uci_remove openvpn "$sec" auth_key_direction && uci_set "openvpn" "$sec" key_direction "${tls_auth##* }"
		tls_auth=${tls_auth% *}
	}
	tls_auth_dir="/etc/vuci-uploads/${tls_auth##*/}"; tls_auth_dir="${tls_auth_dir/.server_/.}"; tls_auth_dir="${tls_auth_dir/.client_/.}"
	[ "$tls_crypt" != "${tls_crypt##* }" ] && tls_crypt=${tls_crypt% *}
	tls_crypt_dir="/etc/vuci-uploads/${tls_crypt##*/}"; tls_crypt_dir="${tls_crypt_dir/.server_/.}"; tls_crypt_dir="${tls_crypt_dir/.client_/.}"
	crl_verify_dir="/etc/vuci-uploads/${crl_verify##*/}"; crl_verify_dir="${crl_verify_dir/.server_/.}"; crl_verify_dir="${crl_verify_dir/.client_/.}"
	secret_dir="/etc/vuci-uploads/${secret##*/}"; secret_dir="${secret_dir/.server_/.}"; secret_dir="${secret_dir/.client_/.}"
	userpass_dir="/etc/vuci-uploads/${userpass##*/}"; userpass_dir="${userpass_dir/.server_/.}"; userpass_dir="${userpass_dir/.client_/.}"

	[ -n "$ca" ] && mv "$ca" "$ca_dir" && uci_set "openvpn" "$sec" "ca" "$ca_dir"
	[ -n "$cert" ] && mv "$cert" "$cert_dir" && uci_set "openvpn" "$sec" "cert" "$cert_dir"
	[ -n "$key" ] && mv "$key" "$key_dir" && uci_set "openvpn" "$sec" "key" "$key_dir"
	[ -n "$dh" ] && mv "$dh" "$dh_dir" && uci_set "openvpn" "$sec" "dh" "$dh_dir"
	[ -n "$tls_auth" ] && mv "$tls_auth" "$tls_auth_dir" && uci_set "openvpn" "$sec" "tls_auth" "$tls_auth_dir"
	[ -n "$tls_crypt" ] && mv "$tls_crypt" "$tls_crypt_dir" && uci_set "openvpn" "$sec" "tls_crypt" "$tls_crypt_dir"
	[ -n "$crl_verify" ] && mv "$crl_verify" "$crl_verify_dir" && uci_set "openvpn" "$sec" "crl_verify" "$crl_verify_dir"
	[ -n "$secret" ] && mv "$secret" "$secret_dir" && uci_set "openvpn" "$sec" "secret" "$secret_dir"
	[ -n "$userpass" ] && mv "$userpass" "$userpass_dir" && uci_set "openvpn" "$sec" "userpass" "$userpass_dir"

	[ "$_role" == "client" ] && \
		uci_set "openvpn" "$sec" "status" "/tmp/openvpn-status_${name}.log"

	uci_rename "openvpn" "$sec" "$name"

}

check_tap(){
        dev=$(uci_get "openvpn" "$1" "dev")

        if [ "$dev" = "tap" ]; then
                is_present="1"
        fi
}

fix_tap(){
        cfg_ifname=$(uci_get "network" "lan" "ifname")
        [ "$is_present" != 1 ] && uci_set "network" "lan" "ifname" "${cfg_ifname// tap0/""}"
}

config_load openvpn
config_foreach check_tap openvpn
fix_tap
config_foreach fix_openvpn openvpn

uci_get openvpn teltonika_auth_service
[ "$?" -eq 0 ] && uci_remove openvpn teltonika_auth_service

uci_commit openvpn
uci_commit network


