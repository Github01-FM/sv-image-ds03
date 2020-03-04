#Copyright (C) 2014 CSR 
DESCRIPTION = "mjpeg for csr platforms"
SECTION = "multimedia"

PACKAGE_ARCH = "${MACHINE_ARCH}"
COMPATIBLE_MACHINE = "(atlas)"

LICENSE = "COMMERCIAL LICENSE"
LIC_FILES_CHKSUM = "file://COPYING;md5=71fa7862d8960f6e15c0545a6d4422c4"

FILESEXTRAPATHS_prepend := "${SRC_URI_mjpeg-path}:"
SRC_URI_mjpeg ?= "git://${CSR_REPO_SITE}/mmgfx/mjpeg/um.git;protocol=http;branch=master"

SRC_URI = "${SRC_URI_mjpeg}"
SRCREV = "${AUTOREV}"

inherit autotools pkgconfig

INSANE_SKIP_${PN} = "staticdev"

INSANE_SKIP_${PN}-dev = "staticdev"

FILES_${PN}-staticdev += "usr/lib/libmjpeg.a"

FILES_${PN}-dbg += " ${libdir}/arm-linux-gnueabihf/.debug"

FILES_${PN}-dev += "usr/include/mjpegpath.h"
FILES_${PN}-dev += "usr/include/Jpeg.h"
FILES_${PN}-dev += "usr/include/jpegtypes.h"
FILES_${PN}-dev += "usr/include/jpegprivate.h"
FILES_${PN}-dev += "usr/include/macros.h"
FILES_${PN}-dev += "usr/include/jpeg_external.h"
FILES_${PN}-dev += "usr/include/pathopen.h"

python () {
        if d.getVar('SRC_URI_mjpeg-path',True) :
                d.setVar('S', "${WORKDIR}/prebuilts/atlas7")
        else:
                d.setVar('S', "${WORKDIR}/git")
}
B = "${S}"

do_configure() {
	export SYSROOT_DIR=${STAGING_DIR_HOST}
	cd ${S} 
	./autogen.sh 
	oe_runmake
}

do_compile_prepend () {
	export SYSROOT_DIR=${STAGING_DIR_HOST}
}

do_install() {
	install -D ${S}/libmjpeg.a ${D}/usr/lib/libmjpeg.a
    install -D ${S}/mjpegpath.h ${D}/usr/include/mjpegpath.h
    install -D ${S}/Jpeg.h ${D}/usr/include/Jpeg.h
    install -D ${S}/jpegtypes.h ${D}/usr/include/jpegtypes.h
    install -D ${S}/macros.h ${D}/usr/include/macros.h
    install -D ${S}/jpegprivate.h ${D}/usr/include/jpegprivate.h
    install -D ${S}/jpeg_external.h ${D}/usr/include/jpeg_external.h
    install -D ${S}/pathopen.h ${D}/usr/include/pathopen.h
}

