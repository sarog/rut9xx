#!/bin/sh

. /lib/functions.sh
. /lib/teltonika-functions.sh

events_file=""
file_name=""
content_type="Content-type: text/plain"
recipients=""
day=$(date +'%m-%d-%y')

send_ftp(){
	local host
	local user
	local password
	local rule="$1"

	config_get host "$rule" "host" ""
	config_get user "$rule" "user" ""
	config_get password "$rule" "password" ""

	host="ftp://$host/"

	curl -T $events_file $host --user "${user}:${password}"
}

format_rec_string(){
	if [ -n "$recipients" ]; then
		recipients="$recipients $1"
	else
		recipients="$1"
	fi
}

send_email(){
	local smtp_host
	local smtp_port
	local subject
	local message
	local user_name
	local password
	local sender
	local secure_conn
	local attachment
	local rule="$1"

	attachment=$(openssl base64 < $events_file)
	config_get smtp_host "$rule" "smtpIP" ""
	config_get smtp_port "$rule" "smtpPort" ""
	config_get subject "$rule" "subject" ""
	config_get message "$rule" "message" ""
	config_get user_name "$rule" "user" ""
	config_get password "$rule" "password" ""
	config_get sender "$rule" "senderEmail" ""
	config_get secure_conn "$rule" "secureConnection" ""
	config_list_foreach "$rule" "recipEmail" format_rec_string

	if [ -z "$user_name" ] || [ -z "$password" ]; then
		user_name=""
		password=""
	fi

	if [ -z "$smtp_host" ] || [ -z "$smtp_port" ] || [ -z "$sender" ]; then
		debug "EXIT"
		return 1
	fi

	cat >/tmp/event_report_email.mime <<EOF
Mime-Version: 1.0
From: $sender
Subject: $subject
Content-type: multipart/mixed; boundary="emailbound"

--emailbound
Content-type: text/plain; charset=utf-8
Content-Transfer-Encoding: quoted-printable

$message
--emailbound
$content_type
Content-Transfer-Encoding: base64
Content-Disposition: attachment; filename="$file_name"

$attachment
--emailbound--

EOF

	if [ "$secure_conn" != "1" ]; then
		cmd="sendmail -S $smtp_host:$smtp_port -f'$sender' -au'$user_name' -ap'$password' $recipients"
	else
		cmd="sendmail -H \"exec openssl s_client -quiet -connect $smtp_host:$smtp_port -tls1 -starttls smtp\" -f'$sender' -au'$user_name' -ap'$password' $recipients"
	fi

	cat /tmp/event_report_email.mime | sh -c "$cmd"
	ret="$?"

	if [ "$ret" = "0" ]; then
		/usr/bin/eventslog -i -t EVENTS -n "Mail" -e "Email was sent to $recipients"
	else
		/usr/bin/eventslog -i -t EVENTS -n "Mail" -e "Email was not sent to $recipients. (Error=$ret)"
	fi
}

config_load eventslog_report

enable=""
event=""
config_get enable "$1" "enable" "0"
config_get event "$1" "event" ""
config_get type "$1" "type" ""
config_get compress "$1" "compress" "0"

if [ "$enable" = 1 ]; then
	case "$event" in
		system)
			events_file="/tmp/system_events_$day.txt"
			/usr/bin/eventslog -p -t EVENTS -f $events_file
			file_name="system_events_$day.txt$compress_type"
			;;
		network)
			events_file="/tmp/network_events_$day.txt"
			/usr/bin/eventslog -p -t connections -f $events_file
			file_name="network_events_$day.txt"
			;;
		all)
			events_file="/tmp/all_events_$day.txt"
			echo "System events" > $events_file
			/usr/bin/eventslog -p -t EVENTS -a $events_file
			echo "Network events" >> $events_file
			/usr/bin/eventslog -p -t connections -a $events_file
			file_name="all_events_$day.txt"
			;;
	esac

	if [ "$compress" = 1 ]; then
		content_type="Content-type: application/x-compressed"
		gzip $events_file
		rm $events_file
		events_file="$events_file.gz"
		file_name="$file_name.gz"
	fi
	case "$type" in
		FTP)
			send_ftp "$1"
			;;
		Email)
			send_email "$1"
			;;
	esac

	rm $events_file
fi
