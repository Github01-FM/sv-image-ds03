#!/bin/sh

readonly ostype=$1
if test -e /dev/nandblk0; then
	devpath=/dev/nandblk0
	readonly sector_size=$(cat /sys/block/nandblk0/queue/hw_sector_size)
else
	if test -e /dev/mmcblk0; then
		devpath=/dev/mmcblk0
		readonly sector_size=$(cat /sys/block/mmcblk0/queue/hw_sector_size)
	else
		echo "unknow boot media"
		exit
	fi
fi

readonly rawdev=${devpath}p1 #raw partition
readonly raw_part_size=36
readonly bootdev=${devpath}p2 # boot partition
if [ "${ostype}" = "android" ]; then
	swapdev=${devpath}p3 # swap partition for hibernation
	rootdev=${devpath}p5 # rootfs partition (android)
	systdev=${devpath}p6 # system partition (android)
	datadev=${devpath}p7 # data partition (android)
	userdev=${devpath}p8 # user partition
	boot_part_size=30
	root_part_size=256
	syst_part_size=256
	data_part_size=1024
	swap_part_size=160
fi
if [ "${ostype}" = "linux" ]; then
	#swapdev=${devpath}p3   # swap    partition for hibernation
	rootdev=${devpath}p3    # root    partition
	svpdev=${devpath}p5     # svp     partition
	storagedev=${devpath}p6 # storage partition 
	datadev=${devpath}p7    # data    partition
	#mapdev=${devpath}p8	# map	  partition 
	logdev=${devpath}p8	# log	  partition	
	sundrydev=${devpath}p9	# sundry  partition	
	userdev=${devpath}p10    # user    partition
	boot_part_size=30
	root_part_size=512
	
	svp_part_size=512
	storage_part_size=256
	data_part_size=32

	#map_part_size=12288
	log_part_size=100
	sundry_part_size=200
	if [ ! -n "$user_size" ]; then
		user_size=1024
	fi	
fi
rootfs_type=ext4

have_valid_partitions()
{
	if [ "${ostype}" = "android" ]; then
		for i in $rawdev $bootdev $rootdev $systdev $datadev $swapdev $userdev;  do
			if [ ! -e $i ]; then
				echo no
				return
			fi
		done
	fi

	if [ "${ostype}" = "linux" ]; then
		for i in $rawdev $bootdev $rootdev $svpdev $storagedev $datadev $userdev $logdev $sundrydev;  do
			if [ ! -e $i ]; then
				echo no
				return
			fi
		done
		
		var=`fdisk -l $rawdev`
		val=`echo ${var%%M*}`
		part_size=`echo ${val##*:}`
		if [[ $part_size -lt $raw_part_size ]]; then
			echo no
			return
		fi
		
				
		var=`fdisk -l $bootdev`
		val=`echo ${var%%M*}`
		part_size=`echo ${val##*:}`
		if [[ $part_size -lt $boot_part_size ]]; then
			echo no
			return
		fi
		
						
		var=`fdisk -l $rootdev`
		val=`echo ${var%%M*}`
		part_size=`echo ${val##*:}`
		if [[ $part_size -lt $root_part_size ]]; then
			echo no
			return
		fi

		var=`fdisk -l $svpdev`
		val=`echo ${var%%M*}`
		part_size=`echo ${val##*:}`
		if [[ $part_size -lt $svp_part_size ]]; then
			echo no
			return
		fi
		
		var=`fdisk -l $storagedev`
		val=`echo ${var%%M*}`
		part_size=`echo ${val##*:}`
		if [[ $part_size -lt $storage_part_size ]]; then
			echo no
			return
		fi
		
		var=`fdisk -l $datadev`
		val=`echo ${var%%M*}`
		part_size=`echo ${val##*:}`
		if [[ $part_size -lt $data_part_size ]]; then
			echo no
			return
		fi

		var=`fdisk -l $logdev`
		val=`echo ${var%%M*}`
		part_size=`echo ${val##*:}`
		if [[ $part_size -lt $log_part_size ]]; then
			echo no
			return
		fi

		var=`fdisk -l $sundrydevdev`
		val=`echo ${var%%M*}`
		part_size=`echo ${val##*:}`
		if [[ $part_size -lt $sundry_part_size ]]; then
			echo no
			return
		fi
	fi

	echo yes
}

#
# create_partition(part_type, part_num, part_size)
#
create_partition()
{
	local part_type=$1
	local part_num=$2
	local part_size=$3

	if [ "$part_size" = "" ]; then
		show_size=+all
	else
		show_size=$part_size
	fi

	echo -n "create_partition($part_type, $part_num, $show_size)..."

	case $part_type in
	p|e|l);;
	*)
		echo $part_type not supported!;;
	esac

	case $part_type in
	p)
		fdisk $devpath << EOF
		n
		$part_type
		$part_num

		$part_size
		wq
EOF
		;;
	e)
		fdisk $devpath << EOF
		n
		$part_type


		wq
EOF
		;;
	l)
		fdisk $devpath << EOF
		n

		$part_size
		wq
EOF
		;;
	esac

	echo "done"
}

#
# format_partition(part_num, fs_type, label)
#
format_partition()
{
	local part_num=$1
	local fs_type=$2
	local label=$3

	echo -n "format_partition($part_num, $fs_type)..."

	for i in {1..5}; do
		if test -b ${devpath}p${part_num}; then
			break
		fi
		sleep 1
	done

	case $fs_type in
	fat32)
		if mkfs.vfat -F 32 ${devpath}p${part_num} -n $label 1>/dev/null 2>/dev/null; then
			echo done
		else
			echo failed!
		fi
		;;
	swap)
		if mkswap ${devpath}p${part_num}  1>/dev/null 2>/dev/null; then
			echo done
		else
			echo failed!
		fi
		;;
	*)
		if mke2fs -T $fs_type -L $label ${devpath}p${part_num} 1>/dev/null 2>&1; then
			echo done
		else
			echo failed!
		fi
		;;
	esac
}

		for i in $rawdev $bootdev $rootdev $svpdev $storagedev $datadev $userdev $logdev $sundrydev;  do
			if [ ! -e $i ]; then
				echo "$i is not exit"
			else
				str1=`fdisk -l $i`
				str2=`echo ${str1%%M*}`
				size=`echo ${str2##*:}`
				echo "$i size:$size M"
			fi
		done

		#echo `have_valid_partitions`
if [ "`have_valid_partitions`" = "no" ]; then
	echo "remove existed partitions ..."
	fdisk $devpath 1>/dev/null 2>&1 << EOF
	d
	1
	d
	3
	d
	4
	wq
EOF
	echo "done"
	if [ "${ostype}" = "android" ]; then
		create_partition p 1 +${raw_part_size}M
		create_partition p 2 +${boot_part_size}M
		create_partition p 3 +${swap_part_size}M

		create_partition e 4
		create_partition l 5 +${root_part_size}M
		create_partition l 6 +${syst_part_size}M
		create_partition l 7 +${data_part_size}M
		create_partition l 8

		format_partition 2 $rootfs_type boot
		format_partition 3 swap
		format_partition 5 $rootfs_type rootfs
		format_partition 6 $rootfs_type system
		format_partition 7 $rootfs_type data
		format_partition 8 $roofs_type user
	fi

	if [ "${ostype}" = "linux" ]; then
		create_partition p 1 +${raw_part_size}M
		create_partition p 2 +${boot_part_size}M
		create_partition p 3 +${root_part_size}M

		create_partition e 4
		create_partition l 5 +${svp_part_size}M
		create_partition l 6 +${storage_part_size}M
		create_partition l 7 +${data_part_size}M
		#create_partition l 8 +${map_part_size}M
		create_partition l 8 +${log_part_size}M
		create_partition l 9 +${sundry_part_size}M
		create_partition l 10 +${user_size}M

		#format_partition 2 $rootfs_type boot
		format_partition 3 $rootfs_type root
		format_partition 5 $rootfs_type svp
		format_partition 6 $rootfs_type storage
		format_partition 7 $rootfs_type data
		#format_partition 8 $rootfs_type map
		format_partition 8 $rootfs_type log
		format_partition 9 $rootfs_type sundry
		format_partition 10 $rootfs_type user
	fi
else
	echo "partitions are existed"
	format_partition 9 $rootfs_type sundry
fi
