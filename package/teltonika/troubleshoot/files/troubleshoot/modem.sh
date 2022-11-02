#!/bin/sh

MEIG_AT_LIST="ATI \
AT+CGATT? \
AT+CGDCONT? \
AT+CGACT? \
AT+CGPADDR \
AT+CFUN? \
AT+CREG? \
AT+CGREG? \
AT+CEREG? \
AT+LCTSN=0,5 \
AT+CGMR \
AT^SYSCFGEX? \
AT+SGCELLINFOEX? \
AT+EFSRW=0,0,\"/nv/item_files/ims/IMS_enable\" \
AT+NVBURS=2"
QUEC_AT_LIST="ATI \
AT+CGATT? \
AT+CGDCONT? \
AT+CGACT? \
AT+CGPADDR \
AT+CFUN? \
AT+QGMR \
AT+QCAINFO \
AT+CVERSION AT+QMBNCFG=\"list\" \
AT+QPRTPARA=4 \
AT+CREG? \
AT+CGREG? \
AT+CEREG? \
AT+QPRTPARA=4 \
AT+QCFG=\"nwscanmodeex\" \
AT+QCFG=\"nwscanmode\" \
AT+QCFG=\"nwscanseq\" \
AT+QCFG=\"dbgctl\" \
AT+QCFG=\"band\""
GSM_CTL_LIST="connstate netstate imei model manuf serial revision imsi simstate pinstate signal cellid \
operator opernum conntype temp network serving neighbour band software"
TMP_GSM_LOG_FILE="/tmp/tmp_gsm_syslog.log"

validate_modem_status() {
	output=$(gsmctl -O "$modem_id" -A "AT")
	case "$output" in
	*OK*)
		;;
	*)
		modem="skip"
	esac
}

generate_collectibles_gsm() {
	local log_file="$3"

	[ "$2" != "id" ] && return

	local modem_id="$1"
	local modem=$(gsmctl -n -m)

	troubleshoot_init_log "MODEM: \"$modem_id\" GSM INFORMATION" "$log_file"
	ubus call gsmd set_debug '{"level":5}'
	logread -f >"$TMP_GSM_LOG_FILE" &
	local logread_PID=$!

	for cmd in ${GSM_CTL_LIST}; do
		echo -ne "$cmd:   \t" >>"$log_file"
		gsmctl -O $modem_id --$cmd >>"$log_file" 2>&1
	done
	sleep 1

	troubleshoot_init_log "MODEM: \"$modem_id\" GSMD AT commands" "$log_file"
	validate_modem_status
	case "$modem" in
	skip)
		troubleshoot_add_log "Modem not responding to AT commands. Skipping.." "$log_file"
		;;
	*SLM750-V*)
		for cmd in ${MEIG_AT_LIST}; do
			echo -ne "$cmd:   \t" >>"$log_file"
			gsmctl -O "$modem_id" -A "$cmd" >>"$log_file" 2>&1
		done
		;;
	*)
		for cmd in ${QUEC_AT_LIST}; do
			echo -ne "$cmd:   \t" >>"$log_file"
			gsmctl -O "$modem_id" -A "$cmd" >>"$log_file" 2>&1
		done
		;;
	esac

	troubleshoot_init_log "MODEM: \"$modem_id\" GSMD log of gsmctl" "$log_file"
	troubleshoot_add_log "$(cat ${TMP_GSM_LOG_FILE})" "$log_file"
	ubus call gsmd set_debug '{"level":1}'

	kill $logread_PID
	rm -f "$TMP_GSM_LOG_FILE"
}

get_modem_id() {
	local log_file=$3
	json_for_each_item generate_collectibles_gsm "$2" "$log_file"
}

get_modems() {
	json_init
	json_load "$(ubus call gsmd get_modems)"
}

modem_hook() {
	local log_file="${PACK_DIR}gsm.log"

	get_modems
	json_for_each_item get_modem_id modems "$log_file"
}

troubleshoot_hook_init modem_hook
