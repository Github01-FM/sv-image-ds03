#!/bin/sh

if [ "$ACTION" = "change" ]; then
	if [ "$NAME" = "hdmi-input" ] && [ "$1" != "nohdmi" ]; then
		if [ -e /sys/class/extcon/hdmi-input/state ]; then
		   state=$(cat /sys/class/extcon/hdmi-input/state)
		   if [ $state = 1 ]; then
			camera_demo -p &
			sleep 1
		   	sample_rate=$(cat /sys/bus/i2c/devices/1-0078/ch7102_audio_rate)
			arecord -Dhw:2,0 -r $sample_rate -c 2 -f S16_LE --buffer-size=8192 | aplay --buffer-size=8192 &
		   else
			kill -2 `ps -o pid,comm | grep 'camera_demo' | awk '{print $1}'`
			kill -9 `ps -o pid,comm | grep 'arecord' | awk '{print $1}'`
			kill -9 `ps -o pid,comm | grep 'aplay' | awk '{print $1}'`
		   fi
		fi
	fi
	if [ "$NAME" = "rearview-demo" ]; then
		if [ -e /sys/class/extcon/rearview-demo/state ]; then
			state=$(cat /sys/class/extcon/rearview-demo/state)
			if [ $state = 1 ]; then
				/test/bin/lcdc0source CVD &
	                else
				kill -2 `ps | grep 'lcdc0source' | awk '{print $1}'`
			fi
		fi
	fi
	if [ "$NAME" = "h2w" ]; then
		if [ -e /sys/class/extcon/h2w/state ]; then
			state=$(cat /sys/class/extcon/h2w/state)
			amixer -q sset 'Left dac to hp left amp' off
			amixer -q sset 'Right dac to hp right amp' off
			amixer -q sset 'Left dac to speaker lineout' off
			amixer -q sset 'Right dac to speaker lineout' off
			if [ $state = 1 ]; then
				amixer -q sset 'Left dac to hp left amp' on
				amixer -q sset 'Right dac to hp right amp' on
			else
				amixer -q sset 'Left dac to speaker lineout' on
				amixer -q sset 'Right dac to speaker lineout' on
			fi
		fi
	fi
fi
