SUMMARY = "QT + csr qt examples + qml"

inherit populate_sdk_base populate_sdk_qt5
TOOLCHAIN_TARGET_TASK += " linux-libc-headers sgx-linux-um-dev"
include csr-image-common.inc

IMAGE_INSTALL_append = " dosfstools"

IMAGE_INSTALL_append = " packagegroup-core-ssh-openssh"
IMAGE_INSTALL_append = " openssh-sftp-server"

LICENSE = "Proprietary"

inherit core-image
