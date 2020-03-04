#!/bin/sh

TARGET_MEDIA=$1
BOOT_PRT=${TARGET_MEDIA}"p2"
BOOT_DIR=/btmp
WIFI_F="wifi.inf"
BT_F="bt.inf"
WIFI_BT_FILES="wifi_bt_files"
GEN_SETTING_SCRIPT="gen_wifi_bt_setting.sh"
WIFI_BT_FILES_PATH="$2/$WIFI_BT_FILES"
GEN_WIFI_BT_SETTING="$WIFI_BT_FILES_PATH/$GEN_SETTING_SCRIPT"

if ! test -d $WIFI_BT_FILES_PATH; then
	echo "no $WIFI_BT_FILES"
	exit
fi

if ! test -e $GEN_WIFI_BT_SETTING; then
	echo "no $GEN_WIFI_BT_SETTING"
	exit
fi

echo "generate WiFi&BT settings"
$GEN_WIFI_BT_SETTING

echo "mount boot partition"
mkdir $BOOT_DIR
mount -t ext4 $BOOT_PRT $BOOT_DIR

echo "copy WiFi&BT inf to $BOOT_DIR(boot partition)"
cp $WIFI_BT_FILES_PATH/$WIFI_F $BOOT_DIR/
cp $WIFI_BT_FILES_PATH/$BT_F $BOOT_DIR/

echo "umount boot partition"
umount $BOOT_DIR
