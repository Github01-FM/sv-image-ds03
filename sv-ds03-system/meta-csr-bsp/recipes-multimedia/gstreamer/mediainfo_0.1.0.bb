DESCRIPTION = "mediainfo for csr platforms"

PACKAGE_ARCH = "${MACHINE_ARCH}"
COMPATIBLE_MACHINE = "(atlas)"

LICENSE = "LGPL"
LIC_FILES_CHKSUM = "file://COPYING;md5=ab1b6dcc912600bf315256144b71e6f0"

DEPENDS = "libav"

FILESEXTRAPATHS_prepend := "${SRC_URI_MediaInfo-path}:"
SRC_URI_MediaInfo ?= "git://${CSR_REPO_SITE}/git/mm/MediaInfo.git;protocol=http;branch=master"

SRC_URI = "${SRC_URI_MediaInfo}"
SRCREV = "${AUTOREV}"

python () {
	if d.getVar('SRC_URI_MediaInfo-path',True) :
		d.setVar('S', "${WORKDIR}/MediaInfo")
	else:
		d.setVar('S', "${WORKDIR}/git") 
}

inherit autotools pkgconfig
INSANE_SKIP_${PN} = "dev-so"

B = "${S}"

do_install() {
        install -D ${S}/lib/.libs/libmediainfo.so.0.0.0 ${D}/usr/lib/libmediainfo.so.0.0.0
        cd ${D}/usr/lib
        ln -sf libmediainfo.so.0.0.0 libmediainfo.so.0
        ln -sf libmediainfo.so.0.0.0 libmediainfo.so
}
