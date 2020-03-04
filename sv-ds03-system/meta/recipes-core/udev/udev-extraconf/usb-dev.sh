#!/bin/sh
#
# Called from udev
#
# Attempt to mount any added block devices and umount any removed devices

USB_DEV="/media/usb_dev"
if [ "$1" = "1" ]; then
	touch "$USB_DEV"
fi
if [ "$1" = "0" ]; then	

	if test -e "$USB_DEV" ; then
		rm "$USB_DEV"
	fi

fi

 
