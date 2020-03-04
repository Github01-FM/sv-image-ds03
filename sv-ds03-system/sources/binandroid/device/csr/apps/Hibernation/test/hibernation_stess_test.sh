#!/bin/sh

report_test_error()
{
	echo "hibernate test failed!($1)"
	exit 1
}

power_key_code=116
down_key_code=20
enter_key_code=23
left_key_code=21
menu_key_code=82
#hibernate_prepare_time=6
wakeup_prepare_time=40
alarm_prepare_time=5
long_press_time=5


if [ "$1" != "" ]
then
	interval=$1
else
	interval=120
fi
echo "hibernate interval: $interval"

if [ "$2" != "" ]
then
times=$2
else
times=0
fi
echo "hibernate times: $times"


#echo "svc power stayon false"
#if ! svc power stayon false
#then
#report_test_error "svc power stayon false"
#fi

#set alarm
echo "am start -W -n com.example.android.apis/.app.AlarmController -a com.csr.action.AUTO_TEST --ez auto_close true --ei interval $interval --es cmd start_repeat"
if ! am start -W -n com.example.android.apis/.app.AlarmController -a com.csr.action.AUTO_TEST --ez auto_close true --ei interval $interval --es cmd start_repeat
then
	report_test_error "am start -W -n com.example.android.apis/.app.AlarmController -a com.csr.action.AUTO_TEST --ez auto_close true --ei interval $interval --es cmd start_repeat"
fi

sleep $alarm_prepare_time
index=0
while [ true ]
do

	let index++
	echo "current times: $index"

	#send power_key down event
	echo "sendevent /dev/input/event1 1 $power_key_code 1"
	if ! sendevent /dev/input/event1 1 $power_key_code 1
	then
		report_test_error "sendevent /dev/input/event1 1 $power_key_code 1"
	fi

	sleep $long_press_time

	#send power_key up event
	echo "sendevent /dev/input/event1 1 $power_key_code 0"
	if ! sendevent /dev/input/event1 1 $power_key_code 0
	then
		report_test_error "sendevent /dev/input/event1 1 $power_key_code 0"
	fi

	sleep 1

	#send down_key event 3 times
	i=0
	while [ $i -lt 3 ]
	do
		echo "input keyevent $down_key_code"
		if ! input keyevent $down_key_code
		then
			report_test_error "input keyevent $down_key_code"
		fi
		sleep 1
		let i++
	done

	#send enter_key event
	echo "input keyevent $enter_key_code"
	if ! input keyevent $enter_key_code
	then
		report_test_error "input keyevent $enter_key_code"
	fi

	sleep 1

	#send left_key event
	echo "input keyevent $left_key_code"
	if ! input keyevent $left_key_code
	then
		report_test_error "input keyevent $left_key_code"
	fi

	sleep 1

	#send enter_key event
	echo "input keyevent $enter_key_code"
	if ! input keyevent $enter_key_code
	then
		report_test_error "input keyevent $enter_key_code"
	fi

	#sleep $hibernate_prepare_time

	echo "wait into hibernation"
	HIBERNATION_STATE=`getprop "sys.hibernating"`
	while [ $HIBERNATION_STATE == "0" ]
	do
        	HIBERNATION_STATE=`getprop "sys.hibernating"`
        	echo $HIBERNATION_STATE
	        sleep 0.2
	done
	
	echo "wait back from hibernation"
	HIBERNATION_STATE=`getprop "sys.hibernating"`
	while [ $HIBERNATION_STATE != "0" ]
	do
        	HIBERNATION_STATE=`getprop "sys.hibernating"`
        	echo $HIBERNATION_STATE
	        sleep 0.2
	done

	#send menu_key event 1 times
	echo "Send menu_key event to unlock screen..."
	if ! input keyevent $menu_key_code
	then
		report_test_error "input keyevent $menu_key_code"
	fi
	echo "Screen Unlocked."

	sleep $wakeup_prepare_time
	if [ $times -ne 0 ] && [ $index -ge $times ]
	then
		break
	fi

done

#echo "svc power stayon ac"
#if ! svc power stayon ac
#then
#report_test_error "svc power stayon ac"
#fi

echo "hibernate test finished!"

#stop alarm
#echo "am start -W -n com.example.android.apis/.app.AlarmController -a com.csr.action.AUTO_TEST --ez auto_close true --ei interval $interval --es cmd stop_repeat"
#if ! am start -W -n com.example.android.apis/.app.AlarmController -a com.csr.action.AUTO_TEST --ez auto_close true --ei interval $interval --es cmd stop_repeat
#then
#report_test_error "am start -W -n com.example.android.apis/.app.AlarmController -a com.csr.action.AUTO_TEST --ez auto_close true --ei interval $interval --es cmd stop_repeat"
#fi

exit 0

