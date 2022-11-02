#!/bin/sh
# Copyright (C) 2021 Teltonika

PACKAGE_FILE="/etc/package_restore.txt"
BACKUP_PACKAGES="/etc/backup_packages/"
FAILED_PACKAGES="/etc/failed_packages"
TIME_OF_SLEEP=10
ROUTERNAME=$(uci -q get system.system.routername)

[ -d "$BACKUP_PACKAGES" ] && {
	packages=$(ls "$BACKUP_PACKAGES"*.ipk)
	mkdir "$BACKUP_PACKAGES""main"

	for i in $packages; do
		tar x -zf "$i" -C "$BACKUP_PACKAGES" ./control.tar.gz
		tar x -zf "$BACKUP_PACKAGES""control.tar.gz" -C "$BACKUP_PACKAGES" ./control
		[ $(grep -c tlt_name "$BACKUP_PACKAGES"control) -eq 0 ] || mv "$i" "$BACKUP_PACKAGES"main
	done

	while [ -e "/var/lock/opkg.lock" ]; do
		sleep "$TIME_OF_SLEEP"
	done
	
	opkg install $(ls "$BACKUP_PACKAGES"main/*.ipk) $(ls "$BACKUP_PACKAGES"*.ipk)

	rm -rf "$BACKUP_PACKAGES" 2> /dev/null
	/etc/init.d/rpcd reload; /etc/init.d/vuci restart
	touch /tmp/vuci/reload_routes
}

[ -s "$PACKAGE_FILE" ] || exit 0

needed_packets=$(cat "$PACKAGE_FILE" | awk '{print $1}')

for i in $needed_packets; do
	[ -s /usr/lib/opkg/info/"$i".control ] && sed -i "/$i/d" "$PACKAGE_FILE"
done

needed_packets=$(cat "$PACKAGE_FILE" | awk '{print $1}')
hotspot_themes=$(echo "$needed_packets" | grep "hs_theme")

[ -z "$hotspot_themes" ] || {
	for i in $hotspot_themes; do
		theme=$(echo "$i" | awk -F "hs_theme_" '{print $2}')
		uci -q delete landingpage."$theme"
	done
	uci -q commit landingpage
}

while [ -e "/var/lock/opkg.lock" ]; do
	sleep "$TIME_OF_SLEEP"
done

while :; do
	needed_packets=$(cat "$PACKAGE_FILE" | awk '{print $1}')
	/bin/opkg --force_feeds /etc/opkg/teltonikafeeds.conf update 2> /dev/null
	available_packages=$(/sbin/opkg-call)

	[ -z "$available_packages" ] || break
	sleep "$TIME_OF_SLEEP"
done

for i in $needed_packets; do
	router_check=0
	exists=$(echo "$available_packages" | grep -wc "Package: $i")
	router=$(echo "$available_packages" | sed -n "/Package: $i$/,/Package:/p" | grep -w "Router:" | awk -F ": " '{print $2}')
	for r in $router; do
		[ $(echo "$ROUTERNAME" | grep -c "$r") -ne 0 ] && router_check=1
	done

	flash_free=$(df -k | grep -w overlayfs | tail -1 | awk '{print $4}')
	flash_free=$((flash_free * 1000))
	pkg_size=$(/sbin/opkg-call "$i")
	[ "$flash_free" -le "$pkg_size" ] && router_check=0
	[ "$exists" -ne 0 -a "$router_check" -ne 0 ] && /bin/opkg install "$i" 2> /dev/null

	[ -s /usr/lib/opkg/info/"$i".control ] || {
		echo "$(cat $PACKAGE_FILE | grep -w -m 1 $i)" >> "$FAILED_PACKAGES"
		sed -i "/$i/d" "$PACKAGE_FILE"
		continue
	}

	pkg_reboot=$(cat /usr/lib/opkg/info/"$i".control | grep -w 'pkg_reboot:' | awk '{print $2}')
	[ "$pkg_reboot" = "1" ] && PKG_REBOOT=1
	sed -i "/$i/d" "$PACKAGE_FILE"
	/etc/init.d/rpcd reload; /etc/init.d/vuci restart
	touch /tmp/vuci/reload_routes
done

[ "$PKG_REBOOT" -eq 1 ] && /etc/init.d/network restart
rm "$PACKAGE_FILE" 2> /dev/null
