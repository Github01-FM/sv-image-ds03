#!/bin/sh

mkdir -p /data/gps
touch /data/gps/gpsmode
echo "" > /data/gps/gpsmode
chmod 777 /data/gps/gpsmode

GPSRF=-$(cat /etc/gps/gpsrf)

if [ "$1" == "-nrealtime" ]; then
GPS_THREAD_POLICY="-nrealtime "
else
GPS_THREAD_POLICY=
fi

RF_TYPE=
FW_VERSION=B116

VERSION_STRING=$(/usr/bin/gpsexe -version)

echo $VERSION_STRING

MATCH_S117=`expr match "$VERSION_STRING" "Gps version V8.0 S117"`
MATCH_S116=`expr match "$VERSION_STRING" "Gps version V8.0 S116"`
MATCH_B116=`expr match "$VERSION_STRING" "Gps version V7.2 B116"`

if [ "$MATCH_S117" == "21" ];then
FW_VERSION=S117
elif [ "$MATCH_S116" == "21" ];then
FW_VERSION=S117
elif [ "$MATCH_B116" == "21" ];then
FW_VERSION=B116
else
FW_VERSION=B116
fi

case $GPSRF in
	"-triglite")
	RF_TYPE="-triglite"
	modprobe sirfsoc_gps rf_type=1
	;;
	"-3iplus-16m")
	RF_TYPE="-3irf1 -spidevnode /dev/spidev3.0"
	modprobe spi_gpio
	modprobe spidev
	modprobe sirfsoc_gps rf_type=2
	;;
	"-3iplus-26m")
	RF_TYPE="-3irf2 -spidevnode /dev/spidev3.0"
	modprobe spi_gpio
	modprobe spidev
	modprobe sirfsoc_gps rf_type=2
	;;
	"-trig")
	RF_TYPE="-trig"
	modprobe sirfsoc_gps rf_type=0
	modprobe sirfsoc_trig
	mknod /dev/trig_sdio c 199 0
	chmod 0666 /dev/trig_sdio
	;;
	"-cd4150")
	RF_TYPE="-cd4150"
	modprobe sirfsoc_gps rf_type=3
	;;
esac


GPSARGS=$GPS_THREAD_POLICY$RF_TYPE" -dspdevnode /dev/gps -socket /data/gps/gpssocket -i /data/gps/agiout -o /data/gps/agiin"
GPSDEV="gps"
 
rm -f /dev/$GPSDEV
major=`grep $GPSDEV /proc/devices | /usr/bin/cut -d " " -f 1`
if [ ! -z $major ] ; then
   	mknod /dev/$GPSDEV c $major 0
fi
   

/usr/bin/gpsexe $GPSARGS > /dev/null &

if [ $FW_VERSION == B116 ] ; then
	sleep 3
	/usr/bin/gpsmc -socket /data/gps/gpssocket > /dev/null &
fi
