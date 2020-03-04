DESCRIPTION = "alsa-plugins"

LICENSE = "LGPLv2.1+"
LIC_FILES_CHKSUM = "file://COPYING;md5=7fbc338309ac38fefcd64b04bb903e34"

PROVIDES += "alsa-plugins"
DEPENDS = "alsa-lib virtual/kernel libsndfile1 json-c dbus pulseaudio tcp-wrappers gdbm"

ALSA_PLUGINS_VERSION = "1.0.27"
ALSA_PLUGINS_SOURCE = "alsa-plugins-${ALSA_PLUGINS_VERSION}.tar.bz2"
ALSA_PLUGINS_SITE = "ftp://ftp.alsa-project.org/pub/plugins"

SRC_URI = "${ALSA_PLUGINS_SITE}/${ALSA_PLUGINS_SOURCE}"
SRC_URI[md5sum] = "ada0163e0e84c787bfc929ad0f3f5cb8"
SRC_URI[sha256sum] = "0bbd0c37c2dd7baf16363afb2e58169ffb0f9c0a70031b3b6235594630f3ab35"

S = "${WORKDIR}/alsa-plugins-${ALSA_PLUGINS_VERSION}"

ALSA_PLUGINS_CONF_OPT = "--enable-pulseaudio	\
			--disable-jack		\
			--disable-samplerate	\
			--disable-avcodec	\
			--with-speex=no \
"

EXTRA_OEMAKE = 'CROSS_COMPILE=${TARGET_PREFIX}'

EXTRA_OECONF = "${ALSA_PLUGINS_CONF_OPT}"

inherit autotools pkgconfig

INSANE_SKIP_${PN} = "dev-so"

FILES_${PN} += "${libdir}/alsa-lib/libasound_*.so"
FILES_${PN}-dbg += "${libdir}/alsa-lib/.debug"
FILES_${PN}-dev += "${libdir}/alsa-lib/*.la"

do_install_append () {
	cd ${D}/usr/lib/alsa-lib/ && rm -f *_arcam_av.so *_oss.so *_speex.so *_upmix.so \
		*_usb_stream.so *_vdownmix.so
	rm -rf ${D}/usr/share/
}


