#!/bin/sh
. /lib/functions.sh
. /lib/upgrade/common.sh

TAR_PATH=/etc/profiles/
export CONFFILES=/tmp/profile.conffiles
EXEPTIONS="etc\/config\/rms_connect_timer etc\/config\/profiles etc\/crontabs\/root etc\/hosts etc\/config\/luci
    etc\/inittab etc\/group etc\/passwd etc\/profile etc\/shadow etc\/shells etc\/sysctl.conf etc\/rc.local  etc\/config\/teltonika"

add_uci_conffiles() {
	local file="$1"
	( find $(sed -ne '/^[[:space:]]*$/d; /^#/d; p' \
		/etc/profiles.conf /lib/upgrade/keep.d/* 2>/dev/null) \
		-type f -o -type l 2>/dev/null;
	  opkg list-changed-conffiles ) | sort -u > "$file"
	return 0
}

remove_exeptions(){
    for val in $EXEPTIONS
    do
        sed -i "/$val/d" "$1"
    done
}

do_save_conffiles() {
	local conf_tar=$1

    [[ -z "${conf_tar}" ]] && return 1
	[ -z "$(rootfs_type)" ] && {
		echo "Cannot save config while running from ramdisk."
		ask_bool 0 "Abort" && exit
		return 0
	}
	add_uci_conffiles "$CONFFILES"
	# Do not save these configs
	remove_exeptions "$CONFFILES"
	echo "/etc/firewall.user" >> "$CONFFILES"
	tar czf "$conf_tar" -T "$CONFFILES" 2>/dev/null
	rm -f "$CONFFILES"
}

create_md5() {
    local md5_file=$1
    [[ -z "${md5_file}" ]] && return 1
    md5sum /etc/config/* /etc/shadow | grep -vE "profiles|rms_connect_timer" > ${md5_file}
}

uci_apply_defaults() {
	mkdir -p /tmp/uci-defaults
	cp -r /rom/etc/uci-defaults/* /tmp/uci-defaults/
	chmod -R +x /tmp/uci-defaults/
	cd /tmp/uci-defaults || return 0
	files="$(ls)"
	[ -z "$files" ] && return 0
	for file in $files; do
		( . "./$(basename $file)" )
		rm -f "$file"
	done
}

apply_config() {
    local md5file=/var/run/config.md5
    rm -rf /var/run/config.check
    mkdir -p /var/run/config.check
    for config in /etc/config/*; do
        file=${config##*/}
        [[ "$file" = "profiles" ]] && continue
        uci show "${file##*/}" > /var/run/config.check/${file}
    done

    [[ -f ${md5file} ]] && {
        for c in `md5sum -c ${md5file} 2>/dev/null| grep FAILED | cut -d: -f1`; do
            echo "apply $c"
            ubus call service event "{ \"type\": \"config.change\", \"data\": { \"package\": \"$(basename ${c})\" }}"
        done
    }

    md5sum /var/run/config.check/* > ${md5file}
    rm -rf /var/run/config.check

    return 0
}

diff(){
    MD5FILE=$1
    [[ -z "$MD5FILE" ]] && return 1
    [[ -f $MD5FILE ]] && {
        for c in `md5sum -c $MD5FILE 2>/dev/null| grep FAILED | cut -d: -f1`; do
            basename "$c"
        done
    }

    return 0
}

if [[ -n "$1" ]]; then
    case "$1" in
        -b)
            do_save_conffiles $2
            exit $?
        ;;
        -m)
            create_md5 $2
            exit $?
        ;;
        -a)
            apply_config $2
            exit $?
        ;;
        -d)
            diff $2
            exit $?
        ;;
		-u)
            uci_apply_defaults
            exit $?
        ;;
    esac

fi
