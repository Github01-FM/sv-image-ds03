#!/bin/sh
#
# Called from udev
# Attemp to mount any added block devices 
# and remove any removed devices
#

MOUNT="/bin/mount"
UMOUNT="/bin/umount"
#FSCK="/sbin/fsck.vfat"
MOUNTOPTION=""
name="`basename "$DEVNAME"`"
. /etc/default.env
. /usr/bin/insmod-tfat.sh

for line in `cat /etc/udev/mount.blacklist | grep -v ^#`
do
	if ( echo "$DEVNAME" | grep -q "$line" )
	then
		# nand boot will not ignore /dev/mmcblk0p1 since external sd card
		if [ "/dev/mmcblk0p1"x = "$line"x ] && [ "`cat /proc/cmdline | grep "nandblk"`" ]; then
			break
		fi
		logger "udev/mount.sh" "[$DEVNAME] is blacklisted, ignoring"
		exit 0
	fi
done
is_on_read_only_partition () {
        DIRECTORY=$1
        dir=`readlink -f $DIRECTORY`
        while true; do
                if [ ! -d "$dir" ]; then
                        echo "ERROR: $dir is not a directory"
                        exit 1
                else
                        for flag in `awk -v dir=$dir '{ if ($2 == dir) { print "FOUND"; split($4,FLAGS,",") } }; \
                                END { for (f in FLAGS) print FLAGS[f] }' < /proc/mounts`; do
                                [ "$flag" = "FOUND" ] && partition="read-write"
                                [ "$flag" = "ro" ] && { partition="read-only"; break; }
                        done
                        if [ "$dir" = "/" -o -n "$partition" ]; then
                                break
                        else
                                dir=`dirname $dir`
                        fi
                fi
        done
        [ "$partition" = "read-only" ] && echo "yes" || echo "no"
}
 
automount() {
	if [ "nandblk0p5"x = "$name"x ] && [ "`cat /proc/cmdline | grep "nandblk"`" ]  && [ "`is_on_read_only_partition /`" = "yes" ]; then
		exit 2
	fi
	if [ "mmcblk0p5"x = "$name"x ] && [ "`cat /proc/cmdline | grep "mmcblk"`" ]  && [ "`is_on_read_only_partition /`" = "yes" ]; then
		exit 2
	fi
	#
	# mount system & data partition
	#
	if [ -n "$name" ];
	then
		if [ ! -d "/media/$name" ];
		then
			mkdir -p "/media/$name"
		fi 
	fi
	if [ "$fs_type" = "vfat" ]; then
		logger "$DEVNAME is with FAT32 filesystem, repaire before mount"
		# $FSCK -a -w $DEVNAME
		MOUNTOPTION="-o utf8"
		fs_type="tfat"
	fi

	if [ "$fs_type" = "exfat" ]; then
		fs_type="texfat"
	fi

	if ! $MOUNT -t $fs_type $MOUNTOPTION $DEVNAME "/media/$name"
	then
		#logger "mount.sh/automount" "$MOUNT -t auto $DEVNAME \"/media/$name\" failed!"
		rm_dir "/media/$name"
	else
		logger "mount.sh/automount" "Auto-mount of [/media/$name] successful"
	fi
}
	
rm_dir() {
	# We do not want to rm -r populated directories
	if test "`find "$1" | wc -l | tr -d " "`" -lt 2 -a -d "$1"
	then
		! test -z "$1" && rm -r "$1"
	else
		logger "mount.sh/automount" "Not removing non-empty directory [$1]"
	fi
}

if [ "$ACTION" = "add" ] && [ -n "$DEVNAME" ]; then

	# According to the file system type for the mount
	case "$ID_FS_TYPE" in
		vfat)
			# Mount vfat as tfat
			fs_type="tfat"
		;;
		*)
			fs_type="$ID_FS_TYPE"
		;;
	esac

                        #name1=${name:0:3}
                        #name2=${name:3}
#                        port1="1-1:1.0/host"
#                        if echo "$DEVPATH" |grep -q "$port1"
#                        then
#                        fi
		    dev=`echo "$DEVNAME" | awk -F '/' '{print $3}'`
		    if test -e "/sys/devices/noc/noc:mediam/17070000.usb/ci_hdrc.1/usb2/2-1/2-1:1.0/"
                then
				ff=`find /sys/devices/noc/noc:mediam/17070000.usb/ci_hdrc.1/usb2/2-1/2-1:1.0/ -name sd*1 | grep $dev`
				if [ 20 -lt ${#ff} ]
				then
			         name="usbstorage0"
					cat /proc/mounts | awk '{print $1}' | grep -q "^$DEVNAME$" || automount
				else
					ff=`find /sys/devices/noc/noc:mediam/17070000.usb/ci_hdrc.1/usb2/2-1/2-1:1.0/ -name sd* | grep $dev`
					if [ 20 -lt ${#ff} ]
					then
					     name="usbstorage0"
						cat /proc/mounts | awk '{print $1}' | grep -q "^$DEVNAME$" || automount
					fi
				fi
            fi

			#hub usb1 port mount
		    if test -e "/sys/devices/noc/noc:mediam/17060000.usb/ci_hdrc.0/usb1/1-1/1-1.1"
			then
				ff=`find /sys/devices/noc/noc:mediam/17060000.usb/ci_hdrc.0/usb1/1-1/1-1.1/ -name sd*1 | grep $dev`
				if [ 20 -lt ${#ff} ]
				then
					name="usbstorage1"
					cat /proc/mounts | awk '{print $1}' | grep -q "^$DEVNAME$" || automount
				else
					ff=`find /sys/devices/noc/noc:mediam/17060000.usb/ci_hdrc.0/usb1/1-1/1-1.1/ -name sd* | grep $dev`
					if [ 20 -lt ${#ff} ]
					then
						name="usbstorage1"
						cat /proc/mounts | awk '{print $1}' | grep -q "^$DEVNAME$" || automount
					fi
				fi
            fi

			#hub usb2 port mount
		    if test -e "/sys/devices/noc/noc:mediam/17060000.usb/ci_hdrc.0/usb1/1-1/1-1.2"
			then
				ff=`find /sys/devices/noc/noc:mediam/17060000.usb/ci_hdrc.0/usb1/1-1/1-1.2/ -name sd*1 | grep $dev`
				if [ 20 -lt ${#ff} ]
				then
					name="usbstorage2"
					cat /proc/mounts | awk '{print $1}' | grep -q "^$DEVNAME$" || automount
				else
					ff=`find /sys/devices/noc/noc:mediam/17060000.usb/ci_hdrc.0/usb1/1-1/1-1.2/ -name sd* | grep $dev`
					if [ 20 -lt ${#ff} ]
					then
						name="usbstorage2"
						cat /proc/mounts | awk '{print $1}' | grep -q "^$DEVNAME$" || automount
					fi
				fi
            fi

			#not hub - usb1 port mount
		    if test -e "/sys/devices/noc/noc:mediam/17060000.usb/ci_hdrc.0/usb1/1-1/1-1:1.0"
			then
				ff=`find /sys/devices/noc/noc:mediam/17060000.usb/ci_hdrc.0/usb1/1-1/1-1:1.0/ -name sd*1 | grep $dev`
				if [ 20 -lt ${#ff} ]
				then
					name="usbstorage1"
					cat /proc/mounts | awk '{print $1}' | grep -q "^$DEVNAME$" || automount
				else
					ff=`find /sys/devices/noc/noc:mediam/17060000.usb/ci_hdrc.0/usb1/1-1/1-1:1.0/ -name sd* | grep $dev`
					if [ 20 -lt ${#ff} ]
					then
						name="usbstorage1"
						cat /proc/mounts | awk '{print $1}' | grep -q "^$DEVNAME$" || automount
					fi
				fi
            fi

	# If the device isn't mounted at this point, it isn't configured in fstab
	
fi



if [ "$ACTION" = "remove" ] && [ -x "$UMOUNT" ] && [ -n "$DEVNAME" ]; then
	for mnt in `cat /proc/mounts | grep "$DEVNAME" | cut -f 2 -d " " `
	# If umount failed because of device busy, try lsof related process and kill
	do
		if ! $UMOUNT -l "$mnt"
		then
			for pr in `lsof "$mnt" | awk '{print $2}'`
			do
				kill -9 $pr
			done
			$UMOUNT $mnt
		fi
		rm -rf $mnt

	done
	
	# Remove empty directories from auto-mounter
	test -e "/media/.automount-$name" && rm_dir "/media/$name"
fi
