#!/bin/sh
# For first time boot, remount rootfs as R/W to collect readahead data. 
if [ ! -f /.readahead* ]; then
        mount -o remount,rw /
fi

