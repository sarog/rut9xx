#! /bin/sh

. /lib/functions.sh                                                                                                          
. /lib/teltonika-functions.sh

recipients=""

format_rec_string(){
	if [ -n "$recipients" ]; then
		recipients="$recipients $1"
	else
		recipients="$1"
	fi
} 

echo "sendig email, file:$1, device:$2"

if [ -z "$1" ] || [ -z "$2" ]; then
	echo "not enough input arguments (1- email data file path, 2-rs device: rs232 or rs485), exiting"
	exit
fi

if [ "$2" == "/dev/rs232" ]; then
	echo "device rs232"
	device="rs232"
else
	echo "device rs485"
	device="rs485"
fi

all_pids=`pidof serial_data_mail_sender.sh`
pidcount=0

echo "all_pids:$all_pids"
for pid in $all_pids; do 
	echo "pid:$pid";
	pidcount=$((pidcount+1))
done

echo "pidcount:$pidcount"

if [ "$pidcount" -gt 5 ]; then
	echo "too many instances, not starting one more"
	exit
fi

echo "getting uci parammeters"

config_load rs
config_list_foreach "$device" "recipEmail" format_rec_string

email_enabled=`uci -q get rs.$device.email_enabled`
subject=`uci -q get rs.$device.subject`
smtpIP=`uci -q get rs.$device.smtpIP`
smtpPort=`uci -q get rs.$device.smtpPort`
secureConnection=`uci -q get rs.$device.secureConnection`
user=`uci -q get rs.$device.user`
password=`uci -q get rs.$device.password`
senderEmail=`uci -q get rs.$device.senderEmail`
email_text=`cat $1`


echo "got from uci:nemail_enabled:$email_enabled"
echo "subject:$subject"
echo "smtpIP:$smtpIP"
echo "smtpPort:$smtpPort"
echo "secureConnection:$secureConnection"
echo "user:$user"
echo "password:$password"
echo "sender email:$senderEmail"
echo "recipients:$recipients"


if [ -z "$smtpIP" ] || [ -z "$smtpPort" ] || [ -z "$user" ] || [ -z "$password" ] || [ -z "$senderEmail" ] || [ -z "$recipients" ]; then
	echo "some necesary parammeters in uci not found, exiting"
	exit
else
	echo "parammeters ok"
fi

if [ -z "$email_text" ]; then
	echo "email file is empty, exiting"
	exit
fi

i=0
while [ $i -lt 3 ]
do
	echo "sending mail $i time"
	if [ "$secureConnection" == "1" ]; then
		echo "Sending secured email"
		`sendmail -H "exec openssl s_client -quiet -connect $smtpIP:$smtpPort -tls1 -starttls smtp" -f $senderEmail -au"$user" -ap"$password" $recipients<<EOF
subject:$subject
from:$senderEmail
$email_text`
		sending_result=$?
		echo "sending_result=$sending_result"
		if [ "$sending_result" == "0" ]; then
			echo "email sent successfully"
			`/usr/bin/eventslog -i -t EVENTS -n "Mail" -e "Email was sent to $recipients"`
			break
		else		
			echo "sending email fail"
		fi
		
	else
		echo "Sending not secured email"
		`sendmail -S $smtpIP:$smtpPort -f $senderEmail -au"$user" -ap"$password" $recipients<<EOF
subject:$subject
from:$senderEmail
$email_text`
		sending_result=$?
		echo "sending_result=$sending_result"
		if [ "$sending_result" == "0" ]; then
			echo "email sent successfully"
			`/usr/bin/eventslog -i -t EVENTS -n "Mail" -e "Email was sent to $recipients"`
			break
		else
			echo "Error, sending email"
		fi
	fi
	
	if [ -z "$3" ]; then
		i=3
	else
		i=$((i+1))
		echo "going to retry in a minute"
		for i in 1 2 3 4 5 6 7 8 9 10
		do
			sleep 6
		done
	fi 
	
done


echo "removing email file"
rm $1
echo "done"
