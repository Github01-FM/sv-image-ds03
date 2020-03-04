include gstreamer1.0-plugins-base.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=c54ce9345727175ff66d17b67ff51f58 \
                    file://common/coverage/coverage-report.pl;beginline=2;endline=17;md5=a4e1830fce078028c8f0974161272607 \
                    file://COPYING.LIB;md5=6762ed442b3822387a51c92d928ead0d \
                   "

SRC_URI = "git://source.codeaurora.org/quic/atlas7/multimedia/gst-plugins-base;protocol=https;branch=freedesktop.org/1.6"
SRC_URI += "file://0001-APBU-10846-gst-discoverer-1.0-cannot-work.patch \
			file://0002-gst-play-support-disp-rect-settings-when-v4l2sink.patch \
			file://0003-Add-Linux-Foundation-Copyright.patch \
			file://0004-Add-mute-unmute-in-gst-play-1.0.patch \
			file://0005-tools-gst-play-add-new-options.patch \
			file://0006-APBU-12094-Design-IMG-MJPEG-playback.patch \
			file://0007-Fix-some-insecure-code.patch \
			file://0008-Add-additional-video-buffer-flags.patch"
S = "${WORKDIR}/git"

SRCREV = "a92ff4f2c57e342dc2b97a79bc3ea234541e3c27"

do_configure_prepend() {
	cd ${S}
	./autogen.sh --noconfigure
	cd ${B}
}

PACKAGES =+ "gst-plugins-base-all"
FILES_gst-plugins-base-all = "${libdir}/gstreamer-1.0/*.so ${bindir}/gst-*"

