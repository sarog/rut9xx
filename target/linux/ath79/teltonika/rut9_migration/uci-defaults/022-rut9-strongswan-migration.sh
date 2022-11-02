#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

ascii2hex(){ a="$@";s=0000000;printf "$a" | hexdump | grep "^$s"| sed s/' '//g| sed s/^$s//;}

move_option() {
	local section="$1"
	local option="$2"
	local new_option="$3"
	local new_section="$4"

	config_get value "$section" "$option"
	[ -n "$value" ] || return 0

	uci_set ipsec "$new_section" "$new_option" "$value"
}


move_list() {
	local value="$1"
        local option="$2"
        local new_section="$3"
                                      
	uci add_list ipsec."$new_section"."$option"="$value"	
}

move_certs() {
	local name="$1"
	config_get ca "$name" ca 0
	config_get cert "$name" cert 0
	config_get key "$name" key 0
	config_get right_cert "$name" right_cert 0

	mkdir -p "/etc/vuci-uploads"

	local ca_dir="/etc/vuci-uploads/cbid.ipsec.${name}.cacertca.crt"
	local cert_dir="/etc/vuci-uploads/cbid.ipsec.${name}.leftcertserver.crt"
	local key_dir="/etc/vuci-uploads/cbid.ipsec.${name}.keyserver.key"
	local rcert_dir="/etc/vuci-uploads/cbid.ipsec.${name}.rightcertserver.crt"

	[ "$ca" = 0 ] || {
		mv "$ca" "$ca_dir" && uci_set ipsec "$name" "cacert" "$ca_dir"
	}
        [ "$cert" = 0 ] || {
		mv "$cert" "$cert_dir" && uci_set ipsec "$name" "leftcert" "$cert_dir"
	}
        [ "$key" = 0 ] || {
		mv "$key" "$key_dir" && uci_set ipsec "$name" "key" "$key_dir"
	}
        [ "$right_cert" = 0 ] || {
		mv "$right_cert" "$rcert_dir" && uci_set ipsec "$name" "rightcert" "$rcert_dir"
	}
	
	uci_set ipsec "$name" authentication_method "x509"
}

create_remote() {
	local section="$1"
	uci set ipsec."$section"=remote

	move_option "$section" enabled enabled "$section"
	uci_set ipsec "$section" crypto_proposal "$section"_ph1
	move_option "$section" right gateway "$section"
	move_option "$section" auth authentication_method "$section"
	uci add_list ipsec."$section".tunnel="$section"_c
	move_option "$section" my_identifier local_identifier "$section"
	move_option "$section" rightid remote_identifier "$section"

	config_get auth "$section" auth 0
	if [ "$auth" = "pubkey" ]; then
		move_certs "$section"
	else
		uci_set ipsec "$section" _multiple_secrets 1
	fi
}

create_connection() {
	local section="$1"	
	local new_section="$section"_c	
	uci set ipsec."$new_section"=connection

	config_get dpdaction "$1" dpdaction 0

	uci_set ipsec "$new_section" crypto_proposal "$section"_ph2
	move_option "$section" auto mode "$new_section"
	move_option "$section" ipsec_type type "$new_section"	
	config_list_foreach "$section" leftsubnet move_list local_subnet "$new_section"
	move_option "$section" leftfirewall local_firewall "$new_section"
	[ "$dpdaction" = "restart" ] && uci set ipsec."$new_section"._dpd=1
	move_option "$section" dpdaction dpdaction "$new_section"
	move_option "$section" dpddelay dpddelay "$new_section"
	move_option "$section" dpdtimeout dpdtimeout "$new_section"
	config_list_foreach "$section" rightsubnet move_list remote_subnet "$new_section"
	move_option "$section" keyexchange keyexchange "$new_section"
	move_option "$section" ikelifetime ikelifetime "$new_section"
	move_option "$section" keylife lifetime "$new_section"
	move_option "$section" aggressive aggressive "$new_section"
	move_option "$section" forceencaps forceencaps "$new_section"
	move_option "$section" rightfirewall remote_firewall "$new_section"
	config_list_foreach "$section" custom move_list custom "$new_section"
}

create_phase1() {
	local section="$1"
	local new_section="$section"_ph1
	uci set ipsec."$new_section"=proposal

	move_option "$section" ike_encryption_algorithm encryption_algorithm "$new_section"
	move_option "$section" ike_authentication_algorithm hash_algorithm "$new_section"
	move_option "$section" ike_dh_group dh_group "$new_section"
}

create_phase2() {                                                                            
        local section="$1"                                                                   
        local new_section="$section"_ph2
	uci set ipsec."$new_section"=proposal             

        move_option "$section" esp_encryption_algorithm encryption_algorithm "$new_section"        
        move_option "$section" esp_hash_algorithm hash_algorithm "$new_section"          
        move_option "$section" esp_pfs_group dh_group "$new_section"                              
} 

create_keys() {
	local section="$1"

	new_section=$(uci add ipsec secret)
	move_option "$section" psk_key secret "$new_section"
	uci set ipsec."$new_section".type=psk
	config_list_foreach "$section" id_selector move_list  id_selector "$new_section"
}

change_section() {
	local section="$1"

	create_remote "$section"
	create_connection "$section"
	create_phase1 "$section"
	create_phase2 "$section"
}

SECTION=
fix_missing() {
        local section="$1"
        SECTION="$1"

	uci rename network."$section".ipaddr=gre_ipaddr
}

fix_remote_id() {
        local section="$1"

        config_get remote_id "$section" id_selector
        [ -n "$remote_id" ] || return 0
        uci_set ipsec "$SECTION"_dmvpn remote_identifier "$remote_id"
}

fix_psk() {
        local section="$1"

        config_get psk_key "$section" secret
        [ -n "$psk_key" ] || return 0
        uci_set ipsec "$SECTION"_dmvpn pre_shared_key "$psk_key"
}

> /etc/config/ipsec

config_load strongswan
config_foreach change_section conn
config_foreach create_keys preshared_keys

uci commit ipsec
rm /etc/config/strongswan

config_load dmvpn
config_foreach fix_missing dmvpn

config_load ipsec
config_foreach fix_remote_id secret
config_foreach fix_psk secret

general=$(uci add ipsec ipsec)
uci_set ipsec "$general" rtinstall_enabled '1'
