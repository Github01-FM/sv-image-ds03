#!/bin/sh
# For first time boot, remount rootfs as R/W to collect readahead data. 
cmdline=`cat /proc/cmdline`
cmdline=${cmdline##*root=/dev/}
boot=`echo ${cmdline} | awk '{ print $1 }'`
len=${#boot}-1
boot=${boot:0:${len}}

if [ ! -f /.readahead* ]; then
        mount -o remount,rw /
fi
