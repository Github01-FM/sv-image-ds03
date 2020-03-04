DESCRIPTION = "gst-omx for csr platforms"
SUMMARY = "gst-omx"
SECTION = "multimedia"

PACKAGE_ARCH = "${MACHINE_ARCH}"
COMPATIBLE_MACHINE = "(atlas)"

LICENSE = "LGPLv2.1+"
LIC_FILES_CHKSUM = "file://COPYING;md5=4fbd65380cdd255951079008b364516c"

DEPENDS = "gstreamer1.0 gstreamer1.0-plugins-base m4"

inherit autotools pkgconfig gettext

SRC_URI = "git://source.codeaurora.org/quic/atlas7/multimedia/gst-omx;protocol=https;branch=freedesktop.org/master"
SRC_URI += "file://0002-Poring-lh04-s-changes-from-vdec-gstreamer.patch \
			file://0003-Porting-lh04-s-changes-for-using-v4l2-buffer.patch \
			file://0004-Change-include-patch-to-SYSROOT.patch \
			file://0005-Add-new-file-config.h-for-gst-omx.patch \
			file://0006-Remove-Makfile.am.patch \
			file://0007-update-some-code-for-import-buffer-mode.patch \
			file://0008-update-_OMX_ASYNC_FILL_BUFFER_-for-import-buffer-mod.patch \
			file://0009-enable-use-buffer-mode.patch \
			file://0010-disable-use-buffer-mode.patch \
			file://0011-Add-caps-for-mpeg2-mpeg4.patch \
			file://0012-set-pool-to-buffer-for-cast-case.patch \
			file://0013-U-V-start-address-aligned-to-16byte.patch \
			file://0014-Refine-the-build-scripts-for-cross-platform.patch \
			file://0015-enable-use-buffer-mode.patch \
			file://0016-Change-omx-lib-from-libomx_vxd.so-to-libomxcore.so.patch \
			file://0017-modify-log-level-for-timestamp.patch \
			file://0018-we-do-not-drop-frame-in-gstomxvideodecoder-plugin-be.patch \
			file://0019-Remove-usr-lib-from-parsed_data_gstomx.conf-because-.patch \
			file://0020-allocate-two-more-buffers-for-rendering-latency.patch \
			file://0021-APBU-10139-gst-play-can-not-play-the-second-stream.patch \
			file://0022-omxvideodec-set-needed-buffers-to-config-pool.patch \
			file://0023-APBU-10214-remove-timeout-workaround.patch \
			file://0024-Change-the-omx-version.patch \
			file://0025-APBU-10242-Update-omx-header-file-from-1.1.2-to-1.2..patch \
			file://0026-set-default-fps-for-invalid-fps.patch \
			file://0027-Bugfix-for-APBU-9938.patch \
			file://0028-set-interlace-mode-to-progressive-APBU-10245-APBU-10.patch \
			file://0029-Use-default-from-IMG.patch \
			file://0030-APBU-10260-Build-error-for-gst-omx.patch \
			file://0031-APBU-10260-Build-error-for-gst-omx.patch \
			file://0032-APBU-10260-Remve-LT_INIT-from-configure.ac.patch \
			file://0033-APBU-10237-Sometimes-Csrplayer-hung-up-when-do-seeki.patch \
			file://0034-APBU-10311-APBU-10314-APBU-10315.patch \
			file://0035-APBU-9939-A7DA-Garbage-screen-occurred-when-playing-.patch \
			file://0036-APBU-10260-Build-error-for-gst-omx.patch \
			file://0037-APBU-10313-A7-H264-Gallery-pop-out-Can-t-play-this-v.patch \
			file://0038-Set-Parameters-of-enableNativeBuffer-for-android-cas.patch \
			file://0039-Revert-Set-Parameters-of-enableNativeBuffer-for-andr.patch \
			file://0040-Modify-log-from-print-to-GST_DEBUG_OBJECT.patch \
			file://0041-support-waylandsink-allocator.patch \
			file://0042-Support-ION-in-gst-omx.patch \
			file://0043-add-omx-mjpeg-video-dec-and-enc.patch \
			file://0044-change-log-info-for-waylandsink-checking.patch \
			file://0045-APBU-10846-gst-discoverer-1.0-cannot-work.patch \
			file://0046-APBU-10729-MPEG2-video-crash-reported-by-Desay.patch \
			file://0047-change-for-422-420-and-thumbnail.patch \
			file://0048-gst-omx-add-alignment-info.patch \
			file://0049-APBU-10788-Poring-VDEC-to-new-DDK-6.2.0_181.patch \
			file://0050-APBU-10842-A7DA-MM-gstreamer-can-not-handle-resoluti.patch \
			file://0051-Add-img-specific-definitions-for-OMX-IL-header-1.2.0.patch \
			file://0052-APBU-10981-A7DA-MM-Some-.ts-video-clip-can-t-playbac.patch \
			file://0053-use-extra-data-to-carry-wayland-ion-buffer-fd.patch \
			file://0054-For-gstomxvideoenc-Store-actual-virtual-buffer-addre.patch \
			file://0055-APBU-11490-bugfix.patch \
			file://0056-Gstreamer-auto-pipeline-support-for-CSR-MJPEG-decode.patch \
			file://0057-Add-Linux-Foundation-Copyright.patch \
			file://0058-Handle-Encoding-from-FileSrc-case.patch \
			file://0059-Update-Linux-Foundation-Copyright-for-mjpeg-files.patch \
			file://0060-APBU-11988-A7DA-Cpu-loading-is-higher-than-before-fo.patch \
			file://0061-for-MJPEG-add-interlace-mode-caps-field-if-the-conte.patch \
			file://0062-APBU-12094-Design-IMG-MJPEG-playback.patch \
			file://0063-omx-call-handle_messages-only-once-in-acquire_buffer.patch \
			file://0064-Fix-insecure-code.patch \
			file://0065-Non-copy-for-v4l2-buffers-when-encoding-with-H-W-MJP.patch \
			file://0066-Correct-vendor-name.patch \
			file://0067-Support-video_full_range_flag-of-h264-stream.patch \
			file://0068-Should-use-absolute-path-to-avoid-confusion.patch"
S = "${WORKDIR}/git"

SRCREV = "b4c7c726ef443cf8a89df26026706e391846bb4a"

do_configure_prepend() {
	cd ${S}
	./autogen.sh --noconfigure
	cd ${B}
}

do_install() {
	install -D -m 0644 ${B}/omx/.libs/libgstomx.so ${D}/usr/lib/gstreamer-1.0/libgstomx.so
	install -D -m 0644 ${S}/parsed_data_gstomx.conf ${D}/home/root/.config/parsed_data_gstomx.conf
	install -D -m 0644 ${S}/unparsed_data_gstomx.conf ${D}/home/root/.config/unparsed_data_gstomx.conf
}

FILES_${PN} += " ${libdir}/gstreamer-1.0/*.so /home/root/.config/parsed_data_gstomx.conf /home/root/.config/unparsed_data_gstomx.conf"
FILES_${PN}-dev += " ${libdir}/gstreamer-1.0/*.la ${libdir}/gstreamer-1.0/*.a"
FILES_${PN}-dbg += " ${libdir}/gstreamer-1.0/.debug/ ${libexecdir}/gstreamer-1.0/.debug/"

