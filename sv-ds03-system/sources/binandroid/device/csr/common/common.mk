
# kernel binary
#
ifeq ($(TARGET_PREBUILT_KERNEL),)
TARGET_PREBUILT_KERNEL := $(LOCAL_PATH)/kernel
endif

PRODUCT_CHARACTERISTICS := tablet

PRODUCT_PACKAGES += \
	libtinyalsa \
	libaudioutils \
	lights.$(TARGET_BOARD_PLATFORM) \
	LiveWallpapersPicker \
        gps.$(TARGET_BOARD_PLATFORM) \
	sensors.$(TARGET_BOARD_PLATFORM) \
	camera.$(TARGET_BOARD_PLATFORM) \
	libmplmpu \
	libmlplatform \
	libmllite \
	libinvensense_hal \
	TSCalibration_CSR \
	libsirfsoc-ril \
	usb_modeswitch \
	goodbye \
	hibernation.sh \
	goodbye_800x480.bmp \
	goodbye_800x600.bmp \
	goodbye_1024x600.bmp \
	audio.primary.sirfsoc \
	audio_policy.sirfsoc \

ifeq ($(TARGET_BUILD_VARIANT),eng)
PRODUCT_PACKAGES += \
	ApiDemos
endif

# keyboard layouts
#
PRODUCT_COPY_FILES += \
	device/csr/common/sirfsoc-keypad.kl:system/usr/keylayout/sirfsoc_touchscreen.kl \

# default apns list
#
PRODUCT_COPY_FILES += \
	device/csr/common/apns-conf.xml:system/etc/apns-conf.xml

# board specific init.rc
#
PRODUCT_COPY_FILES += \
	device/csr/common/init.rc:root/init.rc \
	device/csr/common/init.sirfsoc.rc:root/init.sirfsoc.rc \
	device/csr/common/ueventd.sirfsoc.rc:root/ueventd.sirfsoc.rc \
	#device/csr/common/ueventd.rc:root/ueventd.rc \

#  synergy specific init.rc 
#
synergy_rc := device/common/synergy/init.synergy.rc
ifeq ($(synergy_rc), $(wildcard $(synergy_rc)))
PRODUCT_COPY_FILES += \
	$(synergy_rc):root/init.synergy.rc 
endif

# confs
#
PRODUCT_COPY_FILES += \
	device/csr/common/vold.fstab:system/etc/vold.fstab \
	device/csr/common/binddir.conf:system/etc/binddir.conf \
	device/csr/common/vendor_reserve_process.conf:system/etc/vendor_reserve_process.conf \

# script files
#
PRODUCT_COPY_FILES += \
	device/csr/common/init.gprs-pppd:system/etc/ppp/init.gprs-pppd \
	device/csr/common/ip-up:system/etc/ppp/ip-up \
	device/csr/common/options.gprs:data/ppp/options.gprs \

# properties
#
PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.heapsize=32m

# proclaim OpenGL ES 2.0 support 
PRODUCT_PROPERTY_OVERRIDES += \
    ro.opengles.version=131072

# sirf proprietary code
#
proprietary := $(LOCAL_PATH)/proprietary

PRODUCT_COPY_FILES += \

# Pick up some sounds
#include frameworks/base/data/sounds/AudioPackage2.mk


# Gps sevice script copy
PRODUCT_COPY_FILES += \
   device/csr/common/gps.sh:system/bin/gps.sh

# init.board.sh
#PRODUCT_COPY_FILES += \
   device/csr/common/init.sirfsoc.sh:system/etc/init.sirfsoc.sh

# other scripts/apps
PRODUCT_COPY_FILES += \
   device/csr/common/chelper:system/bin/chelper

# DVD daemon
PRODUCT_COPY_FILES += \
   device/csr/common/dvdd:system/bin/dvdd

# Set default USB interface
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
	persist.sys.usb.config=mass_storage

# These are the hardware-specific features
PRODUCT_COPY_FILES += \
	frameworks/base/data/etc/tablet_core_hardware.xml:system/etc/permissions/tablet_core_hardware.xml \
	frameworks/base/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
	frameworks/base/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
	frameworks/base/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
	frameworks/base/data/etc/android.hardware.touchscreen.multitouch.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.xml \
	frameworks/base/data/etc/android.hardware.sensor.gyroscope.xml:system/etc/permissions/android.hardware.sensor.gyroscope.xml \
        frameworks/base/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
	frameworks/base/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
        frameworks/base/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
        frameworks/base/data/etc/android.hardware.telephony.cdma.xml:system/etc/permissions/android.hardware.telephony.cdma.xml \
	packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:system/etc/permissions/android.software.live_wallpaper.xml

