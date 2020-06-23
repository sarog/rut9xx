#! /bin/sh
mode="$1"
action="$4"
action_select="$2"
action_type="$3"
retry="$5"

if [ "$action" != "sendEmail" ]; then
    if [ "$retry" != "0" ]; then
        count="$6"
        checkint="$7"
        message="$8"
        redun="$9"
        if [ "$redun" != "0" ]; then
            redun_interval="$10"
            phone="$11"
        else
            phone="$10"
        fi
    else
        message="$6"
        redun="$7"
        if [ "$redun" != "0" ]; then
            redun_interval="$8"
            phone="$9"
        else
            phone="$8"
        fi
    fi
else
    if [ "$retry" != "0" ]; then
        count="$6"
        checkint="$7"
        message="$8"
        redun="$9"
        if [ "$redun" != "0" ]; then
            redun_interval="$10"
            subject="$11"
            smtp="$12"
            smtp_port="$13"
            user="$14"
            pass="$15"
            sender="$16"
            reciever="$17"
            secure="$18"
        else
            subject="$10"
            smtp="$11"
            smtp_port="$12"
            user="$13"
            pass="$14"
            sender="$15"
            reciever="$16"
            secure="$17"
        fi
    else
        message="$6"
        redun="$7"
        if [ "$redun" != "0" ]; then
            redun_interval="$8"
            subject="$9"
            smtp="$10"
            smtp_port="$11"
            user="$12"
            pass="$13"
            sender="$14"
            reciever="$15"
            secure="$16"
        else
            subject="$8"
            smtp="$9"
            smtp_port="$10"
            user="$11"
            pass="$12"
            sender="$13"
            reciever="$14"
            secure="$15"
        fi
    fi  
fi

if [ "$mode" = "update" ];then
    if [ "$action_select" != "Mobile data" ]; then
        num=`uci add events_reporting rule`
        uci set events_reporting."$num".event="$action_select"
        uci set events_reporting."$num".action="$action"
        uci set events_reporting."$num".enable='1'
        uci set events_reporting."$num".eventMark="$3"
        uci set events_reporting."$num".eventMarkSf="all"
        if [ "$retry" = "0" ]; then
            if [ "$action" = "sendEmail" ]; then
                uci set events_reporting."$num".emailSend="0"
            else
                uci set events_reporting."$num".smsSend="0"
            fi
        else
            if [ "$action" = "sendEmail" ]; then
                uci set events_reporting."$num".emailSend="1"
                uci set events_reporting."$num".email_count="$count"
                uci set events_reporting."$num".checkint="$checkint"
            else
                uci set events_reporting."$num".sms_count="$count"
                uci set events_reporting."$num".checksmsint="$checkint"
                uci set events_reporting."$num".smsSend="1"
            fi        
        fi
        uci set events_reporting."$num".message="$message"
        uci set events_reporting."$num".enable_block="$redun"
        if [ "$redun" != "0" ]; then
            uci set events_reporting."$num".restrict_time="$redun_interval"
        fi
        if [ "$action" != "sendEmail" ]; then
            uci set events_reporting."$num".telnum="$phone"
        else
            uci set events_reporting."$num".subject="$subject"
            uci set events_reporting."$num".smtpIP="$smtp"
            uci set events_reporting."$num".smtpPort="$smtp_port"
            uci set events_reporting."$num".userName="$user"
            uci set events_reporting."$num".senderEmail="$sender"
            uci add_list events_reporting."$num".recipEmail="$reciever"
            uci set events_reporting."$num".secureConnection="$secure"
        fi
        uci commit
    else
        uci delete data_limit.monitoring_limit
        uci set data_limit.monitoring_limit=monitoring_limit
        uci set data_limit.monitoring_limit."$3"_enb_wrn=1
        uci set data_limit.monitoring_limit."$3"_wrn_limit=$4
        uci set data_limit.monitoring_limit."$3"_wrn_period=$5
        uci set data_limit.monitoring_limit.monitoring=1
        uci set data_limit.monitoring_limit.reported=0
        uci set mdcollectd.config.enabled=1
        if [ "$5" = "month" ];then
            uci set data_limit.monitoring_limit."$3"_wrn_day=$6
        elif [ "$5" = "week" ];then
            uci set data_limit.monitoring_limit."$3"_wrn_weekday=$6
        elif [ "$5" = "day" ];then
            uci set data_limit.monitoring_limit."$3"_wrn_hour=$6
        fi
        uci commit data_limit
        uci commit mdcollectd
        /etc/init.d/limit_guard restart
    fi
elif [ "$mode" = "remove" ];then
    echo "Removing alert"
    for i in ${2//,/ }; do
		echo $i
		if [ ! -z "$i" ]; then
            echo $i
            if [ "$i" = "monitoring_limit" ]; then
                uci delete data_limit.monitoring_limit
                uci commit data_limit;
                echo "+"
            else
                echo "*"
                uci delete events_reporting.@rule["$i"]
                uci commit events_reporting
            fi
        fi
	done
else
    exit 1
fi

