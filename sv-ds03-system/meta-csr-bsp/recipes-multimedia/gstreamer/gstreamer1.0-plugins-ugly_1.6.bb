include gstreamer1.0-plugins-ugly.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=a6f89e2100d9b6cdffcea4f398e37343 \
                    file://tests/check/elements/xingmux.c;beginline=1;endline=21;md5=4c771b8af188724855cb99cadd390068 "

SRC_URI = "git://source.codeaurora.org/quic/atlas7/multimedia/gst-plugins-ugly;protocol=https;branch=freedesktop.org/1.6"
SRC_URI += "file://0001-APBU-11514-A7-RMVB-Playback-stoped-after-seek.patch \
			file://0002-Add-Linux-Foundation-Copyright.patch"
S = "${WORKDIR}/git"

SRCREV = "19d6f0ca51a4095945d7dbcfd9997ec9fc0f616e"

do_configure_prepend() {
	cd ${S}
	./autogen.sh --noconfigure
	cd ${B}
}

PACKAGES =+ "gst-plugins-ugly-all"
FILES_gst-plugins-ugly-all = "${libdir}/gstreamer-1.0/*.so ${bindir}/gst-*"

