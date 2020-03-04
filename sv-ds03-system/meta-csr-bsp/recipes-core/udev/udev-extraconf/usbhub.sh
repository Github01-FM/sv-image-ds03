#!/bin/sh
#
# Rong.Wang@csr.com
#

. /etc/default.env

powerpath=`find /sys -name level`
for level in $powerpath
do
	echo on > $level
done
