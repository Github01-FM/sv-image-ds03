#!/bin/sh
#
# Rong.Wang@csr.com
#

. /etc/default.env

if [ "$2" == "configured" ]; then
	if [ `mount | grep -c "$MOUNT_PARTITION"` -ne 0 ]; then
		umount /media/$MOUNT_PARTITION
	fi
	echo /dev/$MOUNT_PARTITION > /sys/bus/platform/devices/$1/gadget/lun0/file
elif [ "$2" == "suspended" -a `mount | grep -c "$MOUNT_PARTITION"` -eq 0 ]; then
	mkdir -p /media/$MOUNT_PARTITION
	mount /dev/$MOUNT_PARTITION /media/$MOUNT_PARTITION
fi

