FILESEXTRAPATHS_prepend := "${THISDIR}/alsa-state:"

SRC_URI += "file://asound-atlas7.conf"
SRC_URI += "file://asound.state.atlas7"

do_install_append() {
        install -m 0644 ${WORKDIR}/asound-atlas7.conf ${D}${sysconfdir}/asound.conf
        install -m 0644 ${WORKDIR}/asound.state.atlas7 ${D}/var/lib/alsa/asound.state
}

