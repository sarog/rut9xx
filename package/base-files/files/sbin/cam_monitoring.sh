#!/bin/ash

#MAC adresai
CAM1_ON_MAC="00:1E:42:7F:FF:F1"
CAM1_OFF_MAC="00:1E:42:7F:FF:F2"

CAM2_ON_MAC="00:1E:42:7F:FF:F3"
CAM2_OFF_MAC="00:1E:42:7F:FF:F4"

CAM3_ON_MAC="00:1E:42:7F:FF:F5"
CAM3_OFF_MAC="00:1E:42:7F:FF:F6"

CAM4_ON_MAC="00:1E:42:7F:FF:F7"
CAM4_OFF_MAC="00:1E:42:7F:FF:F8"

#Help funkcija
Help() {
echo "Help:"
echo "./Camera_Control <Camera> <New state>"
echo "<Camera> can be CAM1 CAM2 CAM3 CAM4"
echo "<New state> can be ON OFF"
echo "e.g. ./Camera_Control CAM1 ON"
}

ipaddr_uci=`uci get network.lan.ipaddr | awk -F. '{print $4}'`
ipaddr_subnets=`uci get network.lan.ipaddr | awk -F. '{print $1 "." $2 "." $3}'`

if [ "$ipaddr_uci" == "1" ]; then
    ipaddr="$(($ipaddr_uci + 1))"
    ping_ip="$ipaddr_subnets.$ipaddr"
elif [ "$ipaddr_uci" == "255" ]; then
    ipaddr="$(($ipaddr_uci - 1))"
    ping_ip="$ipaddr_subnets.$ipaddr"
else
    ping_ip="$ipaddr_subnets.$ipaddr"
fi


#main
if [ "$1" == "CAM1" ]; then
 if [ "$2" == "ON" ]; then
  nping --arp-type ARP-reply -c 2 -e br-lan --arp-sender-mac $CAM1_ON_MAC --source-mac $CAM1_ON_MAC $ping_ip --source-ip 165.166.168.2 
  echo "Kamera 1 ijungta"
 elif [ "$2" == "OFF" ]; then
  nping --arp-type ARP-reply -c 2 -e br-lan --arp-sender-mac $CAM1_OFF_MAC --source-mac $CAM1_OFF_MAC $ping_ip --source-ip 165.166.168.2  
  echo "Kamera 1 isjungta"
 else
  Help
 fi
elif [ "$1" == "CAM2" ]; then
 if [ "$2" == "ON" ]; then
  nping --arp-type ARP-reply -c 2 -e br-lan --arp-sender-mac $CAM2_ON_MAC --source-mac $CAM2_ON_MAC $ping_ip --source-ip 165.166.168.2  
  echo "Kamera 2 ijungta"
 elif [ "$2" == "OFF" ]; then
  nping --arp-type ARP-reply -c 2 -e br-lan --arp-sender-mac $CAM2_OFF_MAC --source-mac $CAM2_OFF_MAC $ping_ip --source-ip 165.166.168.2
  echo "Kamera 2 isjungta"
 else
  Help
 fi
elif [ "$1" == "CAM3" ]; then
 if [ "$2" == "ON" ]; then
  nping --arp-type ARP-reply -c 2 -e br-lan --arp-sender-mac $CAM3_ON_MAC --source-mac $CAM3_ON_MAC $ping_ip --source-ip 165.166.168.2 
  echo "Kamera 3 ijungta"
 elif [ "$2" == "OFF" ]; then
  nping --arp-type ARP-reply -c 2 -e br-lan --arp-sender-mac $CAM3_OFF_MAC --source-mac $CAM3_OFF_MAC $ping_ip --source-ip 165.166.168.2 
  echo "Kamera 3 isjungta"
 else
  Help
 fi
elif [ "$1" == "CAM4" ]; then
 if [ "$2" == "ON" ]; then
  nping --arp-type ARP-reply -c 2 -e br-lan --arp-sender-mac $CAM4_ON_MAC --source-mac $CAM4_ON_MAC $ping_ip --source-ip 165.166.168.2 
  echo "Kamera 4 ijungta"
 elif [ "$2" == "OFF" ]; then
  nping --arp-type ARP-reply -c 2 -e br-lan --arp-sender-mac $CAM4_OFF_MAC --source-mac $CAM4_OFF_MAC $ping_ip --source-ip 165.166.168.2 
  echo "Kamera 4 isjungta"
 else
  Help
 fi
else
 Help
fi
