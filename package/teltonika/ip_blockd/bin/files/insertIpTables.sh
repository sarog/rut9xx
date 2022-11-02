#!/bin/sh

chain="INPUT"
ip=$1
dest_ip=$2
mask="/$3"
port=$4
option=$5

run(){
  local com="iptables"
  
  if [ "$(check_ip "$ip")" = "ipv6" ]; then
    com="ip6tables"
    mask=""
  fi

  if [ $port = "all" ]; then
    $com $option $chain -s $ip$mask -p tcp -j DROP
  elif [ $dest_ip = "0" ]; then
    $com $option $chain -s $ip$mask -p tcp --destination-port $port -j DROP
  else
    $com $option $chain -s $ip$mask -d $dest_ip$mask -p tcp --destination-port $port -j DROP
  fi
}

check(){
  option="-C"
  run
  retVal=$?
  if [ $retVal = 4 ]; then
    run
   retVal=$?
  fi
  if [ $retVal = 1 ]; then
    option="-I"
    run
  fi
}

check_ip() {
  echo "$1" | awk '!/:/{exit 1}' && echo "ipv6"
}

if [ "$5" = "-D" ]; then
  run
  chain="FORWARD"
  run
else
  check
  chain="FORWARD"
  check
fi
