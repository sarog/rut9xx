#!/bin/ash
# Copyright (C) 2016 Teltonika

total_mtd="$(cat /proc/mtd | tail -1 | grep -o -E -e 'mtd[^:]*' | tr -d 'mtd')"
size_sum=0

sum_mtd()
{
	for i in $(seq 0 $1); do 
		each_mtd="$(cat /sys/class/mtd/mtd$i/name)"
		if [ "$each_mtd" = "rootfs" -o "$each_mtd" = "rootfs_data" -o "$each_mtd" = "kernel" ]; then
			continue
		fi
		each_mtd_size="$(cat /sys/class/mtd/mtd$i/size)"
		size_sum=`expr $size_sum + $each_mtd_size`
	done
	size_sum=`expr $size_sum / 1024 / 1024`
	if [ $size_sum -le 8 ]; then
		printf "8\n"
	elif [ $size_sum -le 16 ]; then
		printf "16\n"
	elif [ $size_sum -le 32 ]; then
		printf "32\n"
	elif [ $size_sum -le 64 ]; then
		printf "64\n"
	elif [ $size_sum -le 128 ]; then
		printf "128\n"
	else
		printf "16\n"
	fi
}
if [ "$total_mtd" != "" ]; then
	sum_mtd $total_mtd
else
	printf "16\n"
fi
