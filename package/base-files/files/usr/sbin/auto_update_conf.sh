#!/bin/sh
# Copyright (C) 2014 Teltonika

. /lib/teltonika-functions.sh

CONFIG_GET="uci -q get auto_update.auto_update"
CONFIG_SET="uci -q set auto_update.auto_update"
CONF_FILE_PATH="/tmp/config.tar.gz"
CA_FILE="/etc/cacert.pem"

EXIT_BAD_USAGE=1
EXIT_DISABLED=2
EXIT_BAD_SERIAL=4
EXIT_BAD_URL=5
EXIT_CURL_ERROR=6
EXIT_SERVER_ERROR=7
EXIT_CONFIG_FILE_NOT_FOUND=8
EXIT_BAD_FILE_SIZE_STRING=9
EXIT_BAD_FREE_RAM_STRING=10
EXIT_NOT_ENOUGH_RAM=11
EXIT_BAD_CONFIG=12
EXIT_BAD_MAC=13
EXIT_CURL_CACERT=14

PrintUsage() {
	echo "Usage: `basename $0` [mode (check, download)]"
#	echo "init - perform first start init (used by service init script)"
	echo "check - check for new Config"
#	echo "forced_check - ignore 'enable' tag and check for FW update"
	echo "download - download new Config from server if it exists"

	exit $EXIT_BAD_USAGE
}

IsNumber() {
	local string
	local result

	string="$1"
	result=1

	case "$string" in
		''|*[!0-9]*)
			result=0
			;;
	esac

	return $result
}

download_conf() {
	local serial
	local mac
	local username
	local password
	local type
	local server_url

	serial=`mnf_info sn`

	IsNumber "$serial"
	if [ $? -eq 0 ]; then
		exit $EXIT_BAD_SERIAL
	fi

	mac=`mnf_info mac`
	if [ ${#mac} -ne 12 ]; then
		exit $EXIT_BAD_MAC
	fi

	username=$($CONFIG_GET.userName)
	password=$($CONFIG_GET.password)
	type="config"

	server_url=$($CONFIG_GET.server_url)

	if [ "$server_url" == "" ]; then
		exit $EXIT_BAD_URL
	fi

	model=`mnf_info name`

	query_string="?type=$type&serial=$serial&mac=$mac&model=$model&action=get_conf_size&username=$username&password=$password&file=cfg"
	temporary_path="/tmp/auto_update_conf_filesize"
	http_code=$(curl --connect-timeout 10 --silent --output $temporary_path --write-out "%{http_code}" --cacert "$CA_FILE" -A "RUT9xx ($serial,$mac) Config update service" "$server_url$query_string" 2>&1)
	curl_exit=$?

	if [ $http_code -ne 200 ]; then
		rm -f $temporary_path
		exit $EXIT_SERVER_ERROR
	fi

	if [ $curl_exit -ne 0 ]; then
		rm -f $temporary_path
		if [ $curl_exit -eq 60 ]; then
			exit $EXIT_CURL_CACERT
		fi
		exit $EXIT_CURL_ERROR
	fi

	fw_size="$(cat /tmp/auto_update_conf_filesize)"
	rm -f $temporary_path

	IsNumber "$fw_size"
	if [ $? -eq 0 ]; then
		exit $EXIT_BAD_FILE_SIZE_STRING
	fi

	if [ "$fw_size" = "error:No file" ]; then
		exit $EXIT_CONFIG_FILE_NOT_FOUND
	fi

	uci -q set auto_update.auto_update.config_size="$fw_size"
	uci commit
	query_string="?type=$type&serial=$serial&mac=$mac&model=$model&action=get_conf&username=$username&password=$password&file=cfg"
	http_code=$(curl --connect-timeout 10 --silent --output "$CONF_FILE_PATH" --write-out "%{http_code}" --cacert "$CA_FILE"  -A "RUT9xx ($serial,$mac) Config update service" "$server_url$query_string" 2>&1)
	curl_exit=$?

	if [ $http_code -ne 200 ]; then
		rm -f $CONF_FILE_PATH
		exit $EXIT_SERVER_ERROR
	fi

	if [ $curl_exit -ne 0 ]; then
		rm -f $CONF_FILE_PATH
		if [ $curl_exit -eq 60 ]; then
			exit $EXIT_CURL_CACERT
		fi
		exit $EXIT_CURL_ERROR
	fi
}

checkForUpdate() {
	local serial
	local mac
	local username
	local password
	local type
	local server_url

	serial=`mnf_info sn`

	IsNumber "$serial"
	if [ $? -eq 0 ]; then
		exit $EXIT_BAD_SERIAL
	fi

	mac=`mnf_info mac`
	if [ ${#mac} -ne 12 ]; then
		exit $EXIT_BAD_MAC
	fi

	username=$($CONFIG_GET.userName)
	password=$($CONFIG_GET.password)
	type="config"

	server_url=$($CONFIG_GET.server_url)

	if [ "$server_url" == "" ]; then
		exit $EXIT_BAD_URL
	fi

	model=`mnf_info name`

	query_string="?type=$type&serial=$serial&mac=$mac&model=$model&action=check_conf&username=$username&password=$password&"
	temporary_path="/tmp/auto_update_conf_check"
	http_code=$(curl --connect-timeout 10 --silent --output /tmp/auto_update_conf_check --write-out "%{http_code}" --cacert "$CA_FILE" -A "RUT9xx ($serial,$mac) Config update service" "$server_url$query_string" 2>&1)
	curl_exit=$?

	if [ $http_code -ne 200 ]; then
		rm -f $temporary_path
		exit $EXIT_SERVER_ERROR
	fi

	if [ $curl_exit -ne 0 ]; then
		rm -f $temporary_path
		if [ $curl_exit -eq 60 ]; then
			exit $EXIT_CURL_CACERT
		fi
		exit $EXIT_CURL_ERROR
	fi

	conf_check="$(cat $temporary_path)"
	rm -f $temporary_path

	if [ "$conf_check" = "error:No file" ]; then
		exit $EXIT_CONFIG_FILE_NOT_FOUND
	fi

	query_string="?type=$type&serial=$serial&mac=$mac&model=$model&action=get_conf_size&username=$username&password=$password&file=cfg"
	temporary_path="/tmp/auto_update_conf_filesize"
	http_code=$(curl --connect-timeout 10 --silent --output $temporary_path --write-out "%{http_code}" --cacert "$CA_FILE" -A "RUT9xx ($serial,$mac) Config update service" "$server_url$query_string" 2>&1)
	curl_exit=$?

	if [ $http_code -ne 200 ]; then
		rm -f $temporary_path
		exit $EXIT_SERVER_ERROR
	fi

	if [ $curl_exit -ne 0 ]; then
		rm -f $temporary_path
		if [ $curl_exit -eq 60 ]; then
			exit $EXIT_CURL_CACERT
		fi
		exit $EXIT_CURL_ERROR
	fi

	fw_size="$(cat $temporary_path)"
	rm -f $temporary_path

	IsNumber "$fw_size"
	if [ $? -eq 0 ]; then
		exit $EXIT_BAD_FILE_SIZE_STRING
	fi

	if [ "$fw_size" = "error:No file" ]; then
		exit $EXIT_CONFIG_FILE_NOT_FOUND
	fi
}

case "$1" in
	download)
		download_conf
		;;
	check)
		checkForUpdate
		;;
	*)
		PrintUsage
		;;
esac
