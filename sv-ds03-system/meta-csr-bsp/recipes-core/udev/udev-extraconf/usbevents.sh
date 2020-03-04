#!/bin/sh
# common to atlas4/5 prima and prima2 platform

. /etc/default.env

if [ "$3" == "device" ]; then 
	if [ "$1" == "usb0" ]; then
		GADGET_DRIVER=$USB0_CLASS_DRIVER
		GADGET_DRIVER_PARAMS=$USB0_DRIVER_PARAMS
	elif [ "$1" == "usb1" ]; then
		GADGET_DRIVER=$USB1_CLASS_DRIVER
		GADGET_DRIVER_PARAMS=$USB1_DRIVER_PARAMS
	else
		echo "invalid parameters"
		exit 1
	fi

	# Handle add/remove events for usb0 device 
	if [ $2 == "remove" ]; then
		if [ `lsmod | grep -c $GADGET_DRIVER` -ne 0 ]; then
			modprobe -r ${GADGET_DRIVER}
		fi
	elif [ $2 == "add" ]; then
		if [ `lsmod | grep -c ${GADGET_DRIVER}` -eq 0 ]; then
			# Insert USB0 class driver first, if not inserted 
			if [ "$1" == "usb1" -a `lsmod | grep -c ${USB0_CLASS_DRIVER}` -eq 0 ]; then
				modprobe ${USB0_CLASS_DRIVER} ${USB0_DRIVER_PARAMS}
			fi
			modprobe ${GADGET_DRIVER} ${GADGET_DRIVER_PARAMS}
		fi
	fi
elif [ $3 == "host" ]; then 
	if [ $2 == "remove" ]; then
		echo $1 $2 $3 is not supported >> /dev/ttySiRF1
	else
		if [ $2 == "add" ]; then
			if [ $1 == "msd" ]; then
				# load msd related modules, if not loaded
				if [ `lsmod | grep -c sg` -eq 0 ]; then
					modprobe sg
				fi
				if [ `lsmod | grep -c scsi_wait_scan` -eq 0 ]; then
					modprobe scsi_wait_scan
				fi
				if [ `lsmod | grep -c sd_mod` -eq 0 ]; then
					modprobe sd_mod
				fi
				if [ `lsmod | grep -c usb_storage` -eq 0 ]; then
					modprobe usb-storage delay_use=0
				fi
				if [ `mount | grep -c "$MOUNT_PARTITION"` -eq 0 ]; then
				        mount /dev/$MOUNT_PARTITION /media/$MOUNT_PARTITION
				fi
			elif [ $1 == "hid" ]; then
				# load hid related modules, if not loaded
				if [ `lsmod | grep -c  usbhid` -eq 0 ]; then
					modprobe usbhid  
				fi
			elif [ $1 == "vendor" ]; then
				# This is for USB-Serila converters. 
				if [ `lsmod | grep -c  pl2303` -eq 0 ]; then
					modprobe pl2303
				fi
				if [ `lsmod | grep -c  cp2101` -eq 0 ]; then
					modprobe cp2101
				fi
			fi
		fi
	fi
else
	echo $1 $2 $3 is not supported
fi

exit 0
