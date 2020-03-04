SUMMARY = "Sound server for Linux and Unix-like operating systems"
HOMEPAGE = "http://www.pulseaudio.org"
AUTHOR = "Lennart Poettering"
SECTION = "libs/multimedia"
LICENSE = "GPLv2+ & LGPLv2.1"
LIC_FILES_CHKSUM = "file://GPL;md5=4325afd396febcb659c36b49533135d4 \
                    file://LGPL;md5=2d5025d4aa3495befef8f17206a5b0a1 \
                    file://src/pulsecore/resampler.h;beginline=4;endline=23;md5=c3d539b93f8c82a1780bfa3cfa544a95"

DEPENDS = "liboil libsamplerate0 libsndfile1 libtool"
DEPENDS += "udev alsa-lib glib-2.0 dbus gconf"
DEPENDS += "json-c gdbm speex libxml-parser-perl-native sbc"
DEPENDS += "tcp-wrappers openssl"

INSANE_SKIP_${PN} += "dev-deps"

FILESEXTRAPATHS_prepend := "${SRC_URI_pulseaudio-path}:"
SRC_URI_pulseaudio_tizen = "git://review.tizen.org/platform/upstream/pulseaudio;branch=tizen_3.0_ivi"
SRC_URI_pulseaudio_csr = "git://shaasigit02/git/linux/external/audiomanager/pulseaudio.git;protocol=http;branch=tizen_3.0_ivi"
SRC_URI = "${SRC_URI_pulseaudio_tizen}"
SRC_URI += "file://0001-add-command-single-user-to-enable-shared-memory.patch \
            file://0002-fix-tizen_3.0_ivi-improper-function-call.patch \
            file://system.pa \
            file://daemon.conf \
            file://pulseaudio.service \
"
SRCREV = "9120bf9099551ec194fe34c059140379c22de166"

python () {
        if d.getVar('SRC_URI_pulseaudio-path',True) :
            d.setVar('S', "${WORKDIR}/pulseaudio")
        else:
            d.setVar('S', "${WORKDIR}/git")

        d.setVar('B', "${S}") 
}

inherit autotools useradd systemd

CPPFLAGS = "-DCSR_IVI_AM -DFASTPATH -DNDEBUG"

#Note: the configure can't recognize the --with-libtool-sysroot, need to set --with-sysroot here to make it work!
EXTRA_OECONF += "--localstatedir=/var \
	--disable-default-build-tests \
	--disable-legacy-database-entry-format \
	--disable-manpages \
    --disable-orc \
	--disable-oss-output \
	--disable-oss-wrapper \
	--disable-esound \
	--disable-avahi \
	--disable-jack \
	--disable-lirc \
	--disable-ipv6 \
	--disable-xen \
    --disable-hal-compat \
	--disable-udev-with-usb-only \
	--disable-bluez5 \
	--disable-bluez4 \
	--disable-nls \
	--disable-x11 \
    --disable-static \
	--enable-bt-a2dp-aptx \
    --enable-alsa \
    --without-fftw \
    --with-database=simple \
	--without-caps \
    --with-udev-rules-dir=${prefix}/lib/udev/rules.d \
    --with-system-user=pulse --with-system-group=pulse \ 
    --with-access-group=pulse-access \
"

USERADD_PACKAGES = "${PN}"
GROUPADD_PARAM_pulseaudio = "pulse"
USERADD_PARAM_pulseaudio = "--system --home /var/run/pulse \
                            --no-create-home --shell /bin/false \
                            --groups audio,pulse --gid pulse pulse"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "pulseaudio.service"

do_configure_prepend () {
	cd ${B}
	echo "${PV}" > ${B}/.tarball-version
	NOCONFIGURE=1 ./bootstrap.sh
}

do_configure () {
	CPPFLAGS="-DCSR_IVI_AM -DFASTPATH -DNDEBUG" oe_runconf 
}

do_install_append() {
    rm -rf ${D}/${libdir}/systemd
    install -d ${D}${systemd_unitdir}/system/
    install -m 0644 ${WORKDIR}/pulseaudio.service ${D}${systemd_unitdir}/system
    install -m 0644 ${WORKDIR}/system.pa ${D}${sysconfdir}/pulse
    install -m 0644 ${WORKDIR}/daemon.conf ${D}${sysconfdir}/pulse
}

FILES_${PN} += "${libdir}/pulse-${PV}/modules \
	${libdir}/pulseaudio \
    /lib/systemd/system/ \
 	/usr/share/ \
	${libdir}/cmake \
" 

FILES_${PN}-dbg += "${libdir}/pulse-${PV}/modules/.debug/ \
	${libdir}/pulse-csr/pulse/.debug/ \
	${libdir}/pulseaudio/.debug/ \
	${libdir}/pulseaudio/pulse/.debug/ \
"

