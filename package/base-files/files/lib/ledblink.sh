
LED_PATH="/sys/class/leds"

led0="signal_bar0"
led1="signal_bar1"
led2="signal_bar2"
led3="signal_bar3"
led4="signal_bar4"
ledg="status_green"
ledr="status_red"

led_on()
{
	bright_path="$LED_PATH/$1/brightness"
	echo "1" > $bright_path
}

led_off()
{
	bright_path="$LED_PATH/$1/brightness"
	echo "0" > $bright_path
}

led_off $ledg
led_off $ledr
led_off $led0
led_off $led1
led_off $led2
led_off $led3
led_off $led4

timelong=100000

while [ 1 ]; do
	led_off $led1
	led_on $led0
		usleep $timelong
	led_off $led0
	led_on $led1
		usleep $timelong
	led_off $led1
	led_on $led2
		usleep $timelong
	led_off $led2
	led_on $led3
		usleep $timelong
	led_off $led3
	led_on $led4
		usleep $timelong

	led_off $led4
	led_on $led3
		usleep $timelong
	led_off $led3
	led_on $led2
		usleep $timelong
	led_off $led2
	led_on $led1
		
done 
