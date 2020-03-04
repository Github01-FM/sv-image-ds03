#!/bin/sh

machine=$(chelper --machine)
lcd_density=$(chelper --density)
screen_size=$(chelper --screen-size)
touch_type=$(chelper --touch-type)
pointercal="/data/misc/pointercal"

if [ ! -z $lcd_density ]; then
	setprop ro.sf.lcd_density $lcd_density
fi

mount -o remount,rw /system
mkdir -p /system/bak
if [ "$machine" = "prima2cb" ]; then
	# prima2 evb 
	mv -f /system/app/TSCalibration.apk /system/bak/
	if [ "$touch_type" = "rtp" ]; then
		# RTP touch
		mv -f /system/app/Calibration.apk /system/bak/
	else
		# CTP touch
		mv -f /system/app/TSCalibration_CSR.apk /system/bak/
	fi

	if [ -e $pointercal ]; then
		awk -F" " '{print $1 > "/proc/sys/dev/ts_device/a0" }' $pointercal
		awk -F" " '{print $2 > "/proc/sys/dev/ts_device/a1" }' $pointercal
		awk -F" " '{print $3 > "/proc/sys/dev/ts_device/a2" }' $pointercal
		awk -F" " '{print $4 > "/proc/sys/dev/ts_device/a3" }' $pointercal
		awk -F" " '{print $5 > "/proc/sys/dev/ts_device/a4" }' $pointercal
		awk -F" " '{print $6 > "/proc/sys/dev/ts_device/a5" }' $pointercal
		awk -F" " '{print $7 > "/proc/sys/dev/ts_device/a6" }' $pointercal
	fi
fi

if [ "$machine" = "taishan" ]; then
	# prima2 taishan
	mv -f /system/app/Calibration.apk /system/bak/
	mv -f /system/app/TSCalibration_test.apk /system/bak/
	if [ "$screen_size" = "8" ]; then
		# taishan-8
		setprop ro.sf.hwrotation 270
	else
		# taishan-9
		mv -f /system/app/TSCalibration.apk /system/bak/
	fi
fi
mount -o remount,ro /system
