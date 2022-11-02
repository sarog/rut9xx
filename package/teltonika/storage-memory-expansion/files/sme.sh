#!/bin/sh

#
# Copyright (C) 2021 Teltonika
#

source /lib/functions.sh

alias logger="logger -s -t sme.sh"
usb_msd_fs="ext2"
usb_msd_label="RUTOS_overlay"
pending_reboot="/tmp/.sme_pending_reboot"
orig_overlay="/rwm"
conf_backup="pre_sme.tgz"

block_info_vars() {
	[ -z "$1" ] && return

	local info=$(block info | grep -m 1 "$1" | tr -d '"')
	echo "DEVICE=${info/:*/} ${info/*: /}"
}

rm_overlay_entry_exit_on_fail() {
	config_get uuid "$1" uuid
	config_get target "$1" target
	config_get is_rootfs "$1" is_rootfs 0
	[[ $is_rootfs -eq 0 && "$target" != "/overlay" && "$uuid" != $UUID ]] && {
		return 0
	}

	uci delete fstab.$1 || {
		logger "failed, couldn't remove mount entry"
		exit 1
	}
}

fstab_entries() {
	config_load fstab
	config_foreach rm_overlay_entry_exit_on_fail mount

	uci batch <<-EOF
		set fstab.rwm="mount"
		set fstab.rwm.device="$(awk -e '/\s\/overlay\s/{print $1}' /etc/mtab)"
		set fstab.rwm.target=$orig_overlay
		set fstab.overlay="mount"
		set fstab.overlay.uuid=$UUID
		set fstab.overlay.target="/overlay"
		set fstab.overlay.sme="1"
		set fstab.log="mount"
		set fstab.log.enabled="0"
		set fstab.log.device="$(awk -e '/\s\/log\s/{print $1}' /etc/mtab)"
		commit fstab
	EOF
}

fs_state_ready() {
	# prevent mount_root:overlay.c:mount_overlay():435 from wiping overlay during pre-init
	local fstate="$1/.fs_state"
	rm -f $fstate
	ln -s 2 $fstate # FS_STATE_READY == 2
}

samba_has_it() {
	local $(block_info_vars $1)
	[ "$MOUNT" ] && grep -q "$MOUNT" /etc/config/samba
}

# Matching an exact _binary_ signature of a legacy SME fstab config
is_legacy_fstab() {
	local uuid="$(sed -n 11p /etc/config/fstab | cut -b 15-50)"
	printf "config 'global'
	option	anon_swap	'0'
	option	anon_mount	'0'
	option	auto_swap	'1'
	option	auto_mount	'1'
	option	delay_root	'5'
	option	check_fs	'0'

config 'mount'
	option	target	'/overlay'
	option	uuid	'%s'
	option	enabled	'1'

" "$uuid" > /tmp/fstab_check
	cmp -s /etc/config/fstab /tmp/fstab_check
	local ret=$?
	rm -f /tmp/fstab_check
	return $ret
}

fix_legacy_fstab() {
	local UUID="$(sed -n 11p /etc/config/fstab | cut -b 15-50)"
	cp /rom/etc/config/fstab /etc/config/fstab

	mkdir -p /mnt/sda1
	mount -t $usb_msd_fs /dev/sda1 /mnt/sda1
	rm -rf /mnt/sda1/* /mnt/sda1/.[!.]* /mnt/sda1/..?*
	fstab_entries
	cp -a /overlay/. /mnt/sda1
	umount /mnt/sda1
	rmdir /mnt/sda1
}

expand() {
	[ -e $pending_reboot ] && {
		logger "failed, pending reboot"
		return 1
	}

	source fmt-usb-msd.sh target 2>&- >&- || {
		logger "failed, couldn't determine target device"
		return 1
	}

	samba_has_it $msd && {
		logger "failed, $msd appears to be in use by samba"
		return 1
	}

	source fmt-usb-msd.sh $usb_msd_fs $usb_msd_label || {
		logger "failed, couldn't format $msd"
		return 1
	}

	local $(block_info_vars $msd)
	[ "$UUID" -a "$MOUNT" ] || {
		logger "failed, couldn't get $msd info"
		return 1
	}

	[ "$TYPE" = $usb_msd_fs ] || {
		logger "failed, expecting $msd to be formatted as $usb_msd_fs"
		return 1
	}

	sysupgrade --create-backup $MOUNT/$conf_backup && chmod -w $MOUNT/$conf_backup || \
		logger "warning: failed to backup config: it won't be restored on shrinkage"

	fstab_entries || {
		logger "failed, couldn't modify fstab"
		return 1
	}

	cp -a /overlay/./ $MOUNT/ || {
		logger "failed, couldn't copy overlay files"
		return 1
	}

	sync
	touch $pending_reboot
	logger "expansion successful. pending reboot"
	return 0
}

shrink() {
	[ -e $pending_reboot ] && {
		logger "failed, pending reboot"
		return 1
	}

	local $(block_info_vars $(uci -q get fstab.overlay.uuid))

	cp $MOUNT/$conf_backup $orig_overlay/sysupgrade.tgz || \
		logger "warning: config backup not found: original config won't be restored"

	rm -f $orig_overlay/upper/etc/config/fstab
	fs_state_ready $orig_overlay

	sync
	touch $pending_reboot
	logger "shrinkage successful. pending reboot"
	return 0
}

is_expanded() {
	[ "$(uci -q get fstab.overlay.sme)" != "1" ] && return 1
	local $(block_info_vars $(uci -q get fstab.overlay.uuid))
	[ "$MOUNT" = "/overlay" ]
}

device_present() {
	source fmt-usb-msd.sh target 2>&- >&- && \
	local $(block_info_vars $msd) && \
	[ "$MOUNT" != "/overlay" ] || {
		echo "not_present"
		exit 1
	}

	samba_has_it $msd && {
		echo "in_use_by_samba"
		exit 2
	}

	echo "present"
	exit 0
}

operation() {
	# intended for webui: to be called every few seconds to get operation status
	local ongoing_file="/tmp/.sme_ongoing"

	case $1 in
		ongoing|success|fail)
			echo $1 > $ongoing_file
		;;
		new)
			[ "$(cat $ongoing_file 2>&-)" = "ongoing" ] && {
				logger "an operation is in progress"
				exit 1
			}

			operation ongoing
		;;
		get)
			[ ! -e $ongoing_file ] && {
				echo "none"
				return 0
			}

			cat $ongoing_file
		;;
		*)
			return 1
		;;
	esac
}

preboot() {
	if is_legacy_fstab; then
		fix_legacy_fstab
		reboot -f
		return 0
	fi

	[ "$(uci -q get fstab.overlay.sme)" = "1" ] || return 0

	local $(block_info_vars "$(uci -q get fstab.overlay.uuid)")
	[ -b "$DEVICE" ] || return 1

	local mnt="/tmp/.tmp_overlay"
	mkdir -p $mnt
	mount -t $usb_msd_fs $DEVICE $mnt || return 2
	cd $mnt

	find -maxdepth 1 -mindepth 1 ! -name "$conf_backup" -exec rm -rf {} \;

	cp -a /overlay/./ ./ || return 3
	fs_state_ready $mnt || return 4

	cd -
	sync
	umount $mnt
	mount_root
	umount /rom/overlay
	return 0
}

case $1 in
	--expand|-e)
		operation new

		if is_expanded; then
			logger "already expanded"
			operation fail
			exit 1
		else
			expand && {
				operation success
				exit 0
			} || {
				operation fail
				exit 1
			}
		fi
	;;
	--shrink|-s)
		operation new

		if is_expanded; then
			shrink && {
				operation success
				exit 0
			} || {
				operation fail
				exit 1
			}
		else
			logger "not expanded, won't shrink"
			operation fail
			exit 1
		fi
	;;
	--status|-t)
		[ "$(operation get)" = "ongoing" ] && {
			echo "in_progress"
			exit 0
		}

		if [ -e $pending_reboot ]; then
			echo "reboot"
		elif is_expanded; then
			echo "expanded"
		else
			echo "unexpanded"
		fi
	;;
	--device-present|-d)
		device_present
	;;
	--operation|-o)
		echo $(operation get)
	;;
	--preboot)
		preboot
	;;
	*)
		echo -e "\
Usage:
	-e, --expand          expands storage if all conditions are met
	-s, --shrink          disable/undo expansion
	-t, --status          reports whether storage is 'expanded', 'unexpanded' or waiting for 'reboot'
	-d, --device-present  reports whether an expansion-eligible USB MSD is present
	-o, --operation       reports status/result of an ongoing/finished expansion or shrinkage
"
		exit 1
	;;
esac
