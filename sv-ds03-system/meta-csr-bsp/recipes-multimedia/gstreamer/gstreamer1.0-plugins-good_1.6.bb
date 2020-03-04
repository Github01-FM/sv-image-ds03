include gstreamer1.0-plugins-good.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=a6f89e2100d9b6cdffcea4f398e37343 \
                    file://common/coverage/coverage-report.pl;beginline=2;endline=17;md5=a4e1830fce078028c8f0974161272607 \
                    file://gst/replaygain/rganalysis.c;beginline=1;endline=23;md5=b60ebefd5b2f5a8e0cab6bfee391a5fe"

SRC_URI = "git://source.codeaurora.org/quic/atlas7/multimedia/gst-plugins-good;protocol=https;branch=freedesktop.org/1.6"
SRC_URI += "file://0001-v4l2-buf-type-V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY-whe.patch \
			file://0002-APBU-10139-gst-play-can-not-play-the-second-stream.patch \
			file://0003-APBU-9911-bugfix.patch \
			file://0004-APBU-10254-A7DA-MM-Show-Green-block-use-CSRPlayer-to.patch \
			file://0005-Multimedia-Set-high-priority-rank-for-v4l2sink-eleme.patch \
			file://0006-APBU-10849-New-feature-V4l2sink-GstVideoOverlay-Inte.patch \
			file://0007-APBU-10867-A7DA-csrplayer-Csrplayer-always-on-pause-.patch \
			file://0008-flacparse-bug-invalid-metadata-is-the-last-one.patch \
			file://0009-Workaround-do-not-add-colorimetry-in-v4l2sink.patch \
			file://0010-v4l2sink-set-default-force-aspect-ratio-FALSE.patch \
			file://0011-Be-compliant-with-vip_capture-dirver-Add-new-field-n.patch \
			file://0012-Bugfix-for-raw-h264-stream-playback.patch \
			file://0013-APBU-10729-MPEG2-video-crash-reported-by-Desay.patch \
			file://0014-APBU-11157-A7DA-MM-Not-smooth-Show-green-block-and-h.patch \
			file://0015-APBU-11496-A7-MPEG4-Playback-stoped-after-seek.patch \
			file://0016-Add-Linux-Foundation-Copyright.patch \
			file://0017-APBU-11727-A7DA-Multimedia-Different-display-mode-fo.patch \
			file://0018-APBU-11741-a7da-MM-V4l2sink-Support-both-software-an.patch \
			file://0019-APBU-11757-bugfix.patch \
			file://0020-APBU-11786-bugfix.patch \
			file://0021-Set-display-aspect-ratio-and-overlay-properties-sepa.patch \
			file://0022-APBU-11786-bugfix.patch \
			file://0023-Enable-io-mode-for-DMABUF-correct-the-codes-in-v4l2b.patch \
			file://0024-Code-Revise-If-buffers-not-came-from-omx-use-copying.patch \
			file://0025-Fix-some-insecure-code.patch \
			file://0026-set-fields-info-in-AVI-odml-header-for-MJPEG-if-the-.patch \
			file://0027-APBU-12094-Design-IMG-MJPEG-playback.patch \
			file://0028-De-interlace-during-camera-preivew.patch \
			file://0029-CSRSupport-68154-bugfix.patch \
			file://0030-APBU-12190-bugfix.patch \
			file://0031-full-range-luma-support.patch"
S = "${WORKDIR}/git"

SRCREV = "84289b981bcce05ea7b17a41fcff029e9e8cd55c"

do_configure_prepend() {
	cd ${S}
	./autogen.sh --noconfigure
	cd ${B}
}

PACKAGES =+ "gst-plugins-good-all"
FILES_gst-plugins-good-all = "${libdir}/gstreamer-1.0/*.so ${bindir}/gst-*"

