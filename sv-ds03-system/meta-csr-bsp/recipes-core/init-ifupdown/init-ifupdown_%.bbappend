FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

inherit systemd
SYSTEMD_SERVICE_${PN} = "networking.service"

SRC_URI += "file://networking.service"
SRC_URI += "file://interfaces.patch-configure"

do_configure_append () {
	cd ${WORKDIR}
	patch -p0 < ${WORKDIR}/interfaces.patch-configure
}

do_install_prepend () {
	install -d ${D}${systemd_unitdir}/system
	install -c -m 0644 ${WORKDIR}/networking.service ${D}${systemd_unitdir}/system
}

