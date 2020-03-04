include gstreamer1.0-libav.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263 \
                    file://COPYING.LIB;md5=6762ed442b3822387a51c92d928ead0d \
                    file://ext/libav/gstav.h;beginline=1;endline=18;md5=a752c35267d8276fd9ca3db6994fca9c \
					 file://gst-libs/ext/libav/LICENSE.md;md5=5c6d1ed56d15ca87ddec48d0c3a2051d \
                    file://gst-libs/ext/libav/COPYING.LGPLv2.1;md5=bd7a443320af8c812e4c18d1b79df004"

SRC_URI = "git://source.codeaurora.org/quic/atlas7/multimedia/gst-libav;protocol=https;branch=freedesktop.org/1.6"
S = "${WORKDIR}/git"

SRCREV = "4ec5a077511f7c39b935ef39e321f91d9dbb99cb"

do_configure_prepend() {
	cd ${S}
	./autogen.sh --noconfigure
	cd ${B}
}

