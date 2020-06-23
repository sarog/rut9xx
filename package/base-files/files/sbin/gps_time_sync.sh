#!/bin/sh
# (C) 2016 Teltonika

logger "Synchronising system time with GPS"
gps_time=`gpsctl -e`

if [ "$gps_time" != "" ] && [ "$gps_time" != "0" ]; then
	date -u -s "$gps_time"
	if [ "$?" = "0" ]; then
		logger "GPS time update successfull!"
	else
		logger "GPS time update failed!"
	fi
else
	logger "Not able to get GPS time"
fi
