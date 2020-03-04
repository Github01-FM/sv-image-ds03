include gstreamer1.0.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=6762ed442b3822387a51c92d928ead0d \
                    file://gst/gst.h;beginline=1;endline=21;md5=e059138481205ee2c6fc1c079c016d0d"

SRC_URI = "git://source.codeaurora.org/quic/atlas7/multimedia/gstreamer;protocol=https;branch=freedesktop.org/1.6"
SRC_URI += "file://0001-APBU-10846-gst-discoverer-1.0-cannot-work.patch \
			file://0002-APBU-11157-A7DA-MM-Not-smooth-Show-green-block-and-h.patch \
			file://0003-Add-Linux-Foundation-Copyright.patch"
S = "${WORKDIR}/git"

SRCREV = "ce147f81dd6d4aecb61b2879b28d6591baccdd2f"

do_configure_prepend() {
	cd ${S}
	./autogen.sh --noconfigure
	cd ${B}
}

INSANE_SKIP_${PN} += " ldflags already-stripped installed-vs-shipped"
