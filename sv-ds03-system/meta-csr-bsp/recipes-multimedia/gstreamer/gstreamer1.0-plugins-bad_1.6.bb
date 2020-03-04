include gstreamer1.0-plugins-bad.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=73a5855a8119deb017f5f13cf327095d \
                    file://gst/tta/filters.h;beginline=12;endline=29;md5=8a08270656f2f8ad7bb3655b83138e5a \
                    file://COPYING.LIB;md5=21682e4e8fea52413fd26c60acb907e5 \
                    file://gst/tta/crc32.h;beginline=12;endline=29;md5=27db269c575d1e5317fffca2d33b3b50"

SRC_URI = "git://source.codeaurora.org/quic/atlas7/multimedia/gst-plugins-bad;protocol=https;branch=freedesktop.org/1.6"
SRC_URI += "file://0001-Bugfix-for-APBU-10448.patch \
			file://0002-Bugfix-for-APBU-10500.patch \
			file://0003-merge-csr-waylandsink-patch-in-1.6.patch \
			file://0004-delay-release-gst_buffer.patch \
			file://0005-map-the-correct-data-for-frame-buffer.patch \
			file://0006-enable-overlay-rendering.patch \
			file://0007-add-wl_output_interface-to-get-output-panel-format.patch \
			file://0008-support-interlace-mode-video.patch \
			file://0009-Add-Linux-Foundation-Copyright.patch \
			file://0010-Fix-some-insecure-code.patch"
S = "${WORKDIR}/git"

SRCREV = "42fbf04611e64b80b3a7ee0185490b303f6f425d"

do_configure_prepend() {
	cd ${S}
	./autogen.sh --noconfigure
	cd ${B}
}

PACKAGES =+ "gst-plugins-bad-all"
FILES_gst-plugins-bad-all = "${libdir}/gstreamer-1.0/*.so ${bindir}/gst-*"

