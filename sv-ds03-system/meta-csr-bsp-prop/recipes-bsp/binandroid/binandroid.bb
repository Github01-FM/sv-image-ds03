# Copyright (C) 2014 CSR
DESCRIPTION = "bootloader for csr platforms"

PROVIDES += "binandroid"

PACKAGE_ARCH = "${MACHINE_ARCH}"
COMPATIBLE_MACHINE = "(atlas)"

LICENSE = "Apachev2"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta-csr-bsp-prop/License-csr.txt;md5=12850ca5c3ea75331f995f57d4bbade1"

FILESEXTRAPATHS_prepend := "${SRC_URI_binandroid-path}:"
SRC_URI_binandroid ?= "git://${CSR_REPO_SITE}/git/linux/external/BINandroid.git;protocol=http;branch=atlas7"

SRC_URI = "${SRC_URI_binandroid}"
SRCREV = "${AUTOREV}"

python () {
	if d.getVar('SRC_URI_binandroid-path',True) :
		d.setVar('S', "${WORKDIR}/binandroid")
	else:
		d.setVar('S', "${WORKDIR}/git")
}

EXTRA_OEMAKE = 'CROSS_COMPILE=${TARGET_PREFIX} CC="${TARGET_PREFIX}gcc ${TOOLCHAIN_OPTIONS}"'

BORADNAME ?= "atlas6_evb"
BOARDNAME_atlas7-arm = "atlas7_evb"
SOCNAME ?= "atlas6"
SOCNAME_atlas7-arm = "atlas7"


do_compile () {
		oe_runmake CC=cc CXX=g++ CFLAGS= updater TARGET_ARCH_VARIANT=armv7-a TARGET_PRODUCT=sirfsocv7
		oe_runmake CC=cc CXX=g++ CFLAGS= signapk TARGET_ARCH_VARIANT=armv7-a
		oe_runmake CC=cc CXX=g++ CFLAGS= recoveryimage TARGET_ARCH_VARIANT=armv7-a TARGET_PRODUCT=sirfsocv7

}
