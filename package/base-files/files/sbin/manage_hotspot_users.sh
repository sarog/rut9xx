#! /bin/sh
. /lib/functions.sh
ERRORS=0
help(){
	echo "HELP:"
	echo " 1st param : update | remove"
	echo " 2st param : hotspot1 | hotspot2 | hotspot3 | hotpost 4 (Choose hotspot)"
	echo " 3st param : username"
	echo " 4st param : password"
	echo " note: you can delete multiple users from same hotspot like:"
	echo "remove test1,test2,test3"
	echo "examples:"
	echo "update hotspot1 test123 toor321"
	echo "remove hotspot1 test123"
	exit 1
}

delete_user(){
	local usr hotsp
	config_get usr $1 "username"
        config_get hotsp $1 "id"
	if [ "$user" = "$usr" ] && [ "$hotspot" = "$hotsp" ]; then
		uci delete coovachilli."$1"
		ERRORS=$((ERRORS + $?))
		uci commit
		ERRORS=$((ERRORS + $?))
	fi	
}

if [ $# -le 2 ];then 
	help
fi

case $1 in
	update)
		action="update"
		;;
	remove)
		action="remove"
		;;
	*)
		help
		;;
esac

case $2 in
	hotspot1)
		hotspot="hotspot1"
		;;
	hotspot2)
		hotspot="hotspot2"
		;;
	hotspot3)
		hotspot="hotspot3"
		;;
	hotspot4)
		hotspot="hotspot4"
		;;
	*)
		help
		;;
esac
if [ "$action" = "update" ];then
	hotspot_number=`echo $hotspot | sed 's/[^0-9]//g'`
	session_section="unlimited$hotspot_number"

	if [ ! -z "$3" ]; then
		user="$3"
	else
		help
	fi
        if [ ! -z "$4" ]; then
                pass="$4"
        else
                help
        fi

	section=`uci add coovachilli users`
	uci "set" "coovachilli."$section".username=""$user"
	ERRORS=$((ERRORS + $?))
	uci "set" "coovachilli."$section".password=""$pass"
        ERRORS=$((ERRORS + $?))
	uci "set" "coovachilli."$section".id=""$hotspot"
        ERRORS=$((ERRORS + $?))
	uci "set" "coovachilli."$section".template=$session_section"
        ERRORS=$((ERRORS + $?))
	uci "commit"
        ERRORS=$((ERRORS + $?))
	if [ $ERRORS -gt 0 ]; then
		exit 1
	else
		exit 0
	fi
fi
if [ "$action" = "remove" ];then
	if [ -z $3]; then
		help
		exit 1
	fi	
	for i in ${3//,/ }; do
		echo $i
		if [ ! -z "$i" ]; then
                	user="$i"
        	else
                	help
        	fi
		config_load "coovachilli"
		config_foreach delete_user "users"
	done
        if [ $ERRORS -gt 0 ]; then
                exit 1
        else
                exit 0
        fi
fi
