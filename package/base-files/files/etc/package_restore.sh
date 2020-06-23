#!/bin/sh
# Copyright (C) 2019 Teltonika

. /lib/teltonika-functions.sh

PACKAGE_FILE="/etc/package_restore.txt"
AVAILABLE_PACKAGES="/tmp/available_packages"
BACKUP_PACKAGES="/etc/backup_packages/"

if [ -d "$BACKUP_PACKAGES" ]; then
	packages=`ls "$BACKUP_PACKAGES"`
	for i in $packages; do
		opkg --force-overwrite --force-depends --nocase install "$BACKUP_PACKAGES""$i"
	done
	rm -rf "$BACKUP_PACKAGES" 2> /dev/null
	rm -fr /tmp/luci-indexcache 2> /dev/null
fi

if [ -f "$PACKAGE_FILE" -a -s "$PACKAGE_FILE" ]; then
	needed_packets=`cat "$PACKAGE_FILE" | awk '{print $1}'`
	for i in $needed_packets; do
		if [ `cat /usr/lib/opkg/status | grep $i | wc -l` -ne 0 ]; then
			sed -i "/$i/d" "$PACKAGE_FILE"
		fi
	done
fi

while [ 1 ]; do
	INSERT=0

	if [ -f "$PACKAGE_FILE" -a -s "$PACKAGE_FILE" ]; then
		rut_fota -p

		if [ -f "$AVAILABLE_PACKAGES" -a -s "$AVAILABLE_PACKAGES" ]; then
			needed_packets=`cat "$PACKAGE_FILE" | awk '{print $1}'`
			for i in $needed_packets; do
                if [ `cat "$AVAILABLE_PACKAGES" | grep "$i" | wc -l` -gt 0 ]; then
                    pkg_link=`cat "$AVAILABLE_PACKAGES" | grep "$i" | awk -F " - " '{print $4}'`
                    rm /tmp/tlt_custom_pkg_restore.ipk 2>/dev/null
                    /usr/bin/curl -y 30 -o /tmp/tlt_custom_pkg_restore.ipk "$pkg_link"
                    if [ -f /tmp/tlt_custom_pkg_restore.ipk ]; then
                        opkg --force-removal-of-dependent-packages --force-overwrite --force-depends --nocase install /tmp/tlt_custom_pkg_restore.ipk
                        if [ `cat /usr/lib/opkg/status | grep $i | wc -l` -eq 0 ]; then
                            name=`cat "$PACKAGE_FILE" | grep "$i"`
                            echo "$name" >> /etc/failed_packages
                        else
                            /sbin/reload_config
                            files=`cat /usr/lib/opkg/info/"$i".list`
                            for y in $files; do
                                if [ `echo "$y" | grep "/etc/init.d/" | wc -l` -gt 0 ]; then
                                    "$y" restart
                                    "$y" enable
                                fi
                            done
                            rm -fr /tmp/luci-indexcache 2> /dev/null
                        fi
                        sed -i "/$i/d" "$PACKAGE_FILE"
                    else
                        INSERT=1
                    fi
                else
                    INSERT=1
                fi
			done
		else
			INSERT=1
		fi
	fi

	if [ "$INSERT" -eq 1 ]; then
		sleep 10
	else
		rm -f /etc/uci-defaults/70_custom_opkg
		break
	fi
done
