# This file includes all definitions that apply only to sirfsocv7 devices
#
# Anything that is generic to all products should go in the csr/common directory
#
# Everything in this directory will become public
DEVICE_PACKAGE_OVERLAYS := device/csr/sirfsocv7/overlay

$(call inherit-product, device/csr/common/common.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full.mk)

# Overrides
PRODUCT_NAME := sirfsocv7
PRODUCT_DEVICE := sirfsocv7
PRODUCT_MANUFACTURER := CSR
PRODUCT_LOCALES += ldpi mdpi hdpi

# ro.product.model
PRODUCT_MODEL := SiRFSoC Android
# ro.product.brand
PRODUCT_BRAND := CSR
# ro.board.platform
TARGET_BOARD_PLATFORM := sirfsoc
