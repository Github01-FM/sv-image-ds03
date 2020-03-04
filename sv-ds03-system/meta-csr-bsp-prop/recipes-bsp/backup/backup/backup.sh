#!/bin/sh
### BEGIN INIT INFO
# Provides:          Backup
# Description:       CSR Backup
# Required-Start:    mountvirtfs $local_fs
# Required-Stop:     $local_fs
### END INIT INFO


do_start() {
echo V > /dev/watchdog

##############################################################
# 1. check boot status, change image index if last booting failed
# 2. clear boot status
dd if=/proc/boot_status of=/tmp/data bs=6 count=1 1>/dev/null 2>&1
scratchpad3=`cat /tmp/data`
kernel_booting=${scratchpad3:0:1}
kernel_booting_fail=${scratchpad3:1:1}
recovery_booting=${scratchpad3:2:1}
recovery_booting_fail=${scratchpad3:3:1}

boot_dir=/boot
bootmedia=`cat /proc/cmdline | awk -F "root=" '{print $2}' | cut -f1 -d "p" | cut -f3 -d "/"`
boot_part=$bootmedia"p2"

if test -e /dev/block/$boot_part; then
	boot_part="/dev/block/$boot_part"
else
	boot_part="/dev/$boot_part"
fi

if [ "${scratchpad3}" != "000000" ]; then
	if [ "$kernel_booting_fail" = "1" ] || [ "$recovery_booting_fail" = "1" ]; then

		cmdline="`cat /proc/cmdline`"
		kernel_index="`echo ${cmdline#*kernel_index=} | cut -f1 -d " "`"
		recovery_index="`echo ${cmdline#*recovery_index=} | cut -f1 -d " "`"

		mkdir $boot_dir
		mount -t ext4 $boot_part $boot_dir
		if [ "$kernel_booting_fail" = "1" ]; then
			echo "kernel booting failed last time"
			echo -n "update kernel index ..."
			sed -i '/kernel_index/ s/^.*$/kernel_index='$kernel_index';/' $boot_dir/boot.cfg
			echo "done"
		fi
		if [ "$recovery_booting_fail" = "1" ]; then
			echo "recovery booting failed last time"
			echo -n "update recovery index ..."
			sed -i '/recovery_index/ s/^.*$/recovery_index='$recovery_index';/' $boot_dir/boot.cfg
			echo "done"
		fi
		umount $boot_dir
	fi
	echo "000000" > /proc/boot_status
fi
}

do_stop() {
	echo E > /dev/watchdog
}

######################################################
case "$1" in
start)
	do_start
	;;

stop)
	#do_stop
	;;
esac

exit 0

