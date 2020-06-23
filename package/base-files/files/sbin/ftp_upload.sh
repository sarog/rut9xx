#!/bin/sh
# Upload file to FTP server

APP_NAME="ftp_upload.sh"
CONFIG_GET="uci get tcplogger.ftp"
LOG_FILE="/tmp/ftp_log"

HOST=`$CONFIG_GET.host 2> /dev/null`
LOG_LEVEL=`$CONFIG_GET.log_level 2> /dev/null`
SOURCE_NAME=`$CONFIG_GET.sname 2> /dev/null`
DEST_NAME=`$CONFIG_GET.dname 2> /dev/null`
USER_NAME=`$CONFIG_GET.user 2> /dev/null`
PASSWORD=`$CONFIG_GET.psw 2> /dev/null`
PORT=`$CONFIG_GET.port 2> /dev/null`
DEBUG=`$CONFIG_GET.debug 2> /dev/null`
DELAY=`$CONFIG_GET.delay 2> /dev/null`
EXTRA_NAME_INFO=`CONFIG_GET.extra_name_info 2> /dev/null`
CUSTOM_STRING=`CONFIG_GET.custom_string 2> /dev/null`

if [ -z $LOG_LEVEL ]; then
	LOG_LEVEL=5
fi

if [ -z $DELAY ]; then
	DELAY=6
fi

LOGGER="logger -p $LOG_LEVEL -t $APP_NAME -s"

if [ -z $HOST ]; then
	$LOGGER "No FTP host provided."
	exit 1
fi

if [ -z $SOURCE_NAME ]; then
	#$LOGGER "No source file name provided."
	#exit 1
	SOURCE_NAME="/tmp/wifitracker.log /tmp/smsotp.log"
fi

if [ -z $DEST_NAME ]; then
	#$LOGGER "No destination file name provided."
	#exit 1
	DEST_NAME="wifitracker.tar.gz"
fi


EXTRA_NAME_INFO=`$CONFIG_GET.extra_name_info 2> /dev/null`
CUSTOM_STRING=`$CONFIG_GET.custom_string 2> /dev/null`
EXTRA=""

if [ "$EXTRA_NAME_INFO" == "mac" ]; then
	EXTRA=`mnf_info mac 2> /dev/null`
	EXTRA="$EXTRA""_"
elif [ "$EXTRA_NAME_INFO" == "serial" ]; then
	EXTRA=`mnf_info sn 2> /dev/null`
	EXTRA="$EXTRA""_"
elif [ "$EXTRA_NAME_INFO" == "custom" ]; then
	if [[ ! -z "$CUSTOM_STRING" ]]; then
		EXTRA="$CUSTOM_STRING""_"
	fi
fi

prefix=`date +20%y%m%d_%H%M%S`
DEST_NAME="$prefix""_""$EXTRA""$DEST_NAME"
COMMAND="ftpput"

if [ $USER_NAME ]; then
	COMMAND="$COMMAND -u $USER_NAME"
fi

if [ $PASSWORD ]; then
	COMMAND="$COMMAND -p $PASSWORD"
fi

if [ $PORT ]; then
	COMMAND="$COMMAND -P $PORT"
fi

if [ $DEBUG ]; then
	COMMAND="$COMMAND -v"
fi

if [ $DEBUG ]; then
	COMMAND="$COMMAND -v"
fi

if [ $HOST ]; then
	COMMAND="$COMMAND $HOST"
fi

if [ $DEST_NAME ]; then
	COMMAND="$COMMAND $DEST_NAME"
fi

if [ "$SOURCE_NAME" ]; then
	COMMAND="$COMMAND /tmp/$DEST_NAME"
fi

tar -czf /tmp/$DEST_NAME $SOURCE_NAME
rm -rf $SOURCE_NAME

echo $COMMAND

for i in 1 2 3 4 5
do
	$COMMAND &> $LOG_FILE

	if [ -s $LOG_FILE ]; then
		cat $LOG_FILE | $LOGGER
		sleep $DELAY
	else
		$LOGGER "FTP upload successful."
		rm -rf "/tmp/$DEST_NAME"
		exit 0
	fi
done

rm -rf "/tmp/$DEST_NAME"
$LOGGER "FTP upload failed."

