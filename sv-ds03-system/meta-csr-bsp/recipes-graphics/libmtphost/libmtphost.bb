SECTION = "libs"
PACKAGE_ARCH = "${MACHINE_ARCH}"
COMPATIBLE_MACHINE = "(atlas)"
LICENSE = "LGPLv2.1+"
LIC_FILES_CHKSUM = "file://COPYING;md5=0448d3676bc0de00406af227d341a4d1"

DEPENDS = "libusb1 libglibmtp libgthreadmtp"

SRC_URI = "git://git.code.sf.net/p/libmtp/code;protocol=http"
SRCREV = "1b9f16473127ee7160559141746c8bfbc4a3369b"

FILESEXTRAPATHS_prepend := "${CSR_SOURCE_REPO}/prebuilts/linux-arm/atlas7/mtp:"
BBCLASSEXTEND = "native nativesdk"
SRC_URI += "file://mtpfs"
SRC_URI += "file://mtpprobe"

S = "${WORKDIR}/git"
python () {
		d.setVar('B', "${S}")
}

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
SRC_URI += " \
    file://libmtp.patch \
"

do_compile(){
    cd ${S};
    make;
}

do_install () {
	install -d ${D}/usr/lib
	install ${S}/libmtphost.so.1.0 ${D}/usr/lib/libmtphost.so.1.0
	cd ${D}/usr/lib
	ln -sf libmtphost.so.1.0 libmtphost.so.1
	ln -sf libmtphost.so.1 libmtphost.so
	install -d ${D}${bindir}/
	install ${WORKDIR}/mtpfs ${D}${bindir}
	install ${WORKDIR}/mtpprobe ${D}${bindir}
}
