SECTION = "libs"
PACKAGE_ARCH = "${MACHINE_ARCH}"
COMPATIBLE_MACHINE = "(atlas)"
LICENSE = "LGPL-2.0"
LIC_FILES_CHKSUM = "file://COPYING;md5=3bf50002aefd002f49e7bb854063f7e7"

DEPENDS = "libglibmtp"

BBCLASSEXTEND = "native nativesdk"

SRC_URI = "git://git.gnome.org/browse/glib;protocol=https"
SRCREV = "1cb4af3e96c940a34b9d833ed5d69c9578ba6bd4"

S = "${WORKDIR}/git"
python () {
		d.setVar('B', "${S}")
}

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
SRC_URI += " \
    file://gthread.patch \
"

do_compile(){
    cd ${S}/gthread;
    make;
}

do_install () {
	install -d ${D}/usr/lib
	install ${S}/gthread/libgthread.so.1.0 ${D}/usr/lib/libgthread.so.1.0
	cd ${D}/usr/lib
	ln -sf libgthread.so.1.0 libgthread.so.1
	ln -sf libgthread.so.1 libgthread.so
}
