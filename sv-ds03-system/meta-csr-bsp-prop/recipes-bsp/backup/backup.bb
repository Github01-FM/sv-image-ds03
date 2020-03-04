DESCRIPTION = "backup for csr platforms"

PACKAGE_ARCH = "${MACHINE_ARCH}"
COMPATIBLE_MACHINE = "(atlas)"

LICENSE = "Proprietary"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta-csr-bsp-prop/License-csr.txt;md5=12850ca5c3ea75331f995f57d4bbade1"

SRC_URI += "file://backup.sh"
SRC_URI += "file://backup.service"
SRCREV = "${AUTOREV}"

inherit systemd
SYSTEMD_SERVICE_${PN} = "backup.service"

do_install () {
	#install -d ${D}${bindir}/
	#install ${WORKDIR}/build/launcher ${D}${bindir}
	install -d ${D}/usr/bin
	install -m 0755 ${WORKDIR}/backup.sh ${D}/usr/bin/backup.sh

	install -d ${D}${systemd_unitdir}/system
	install -m 0644 ${WORKDIR}/backup.service ${D}${systemd_unitdir}/system

	#install -d ${D}${systemd_unitdir}/system
	#install -c -m 0644 ${WORKDIR}/launcher.service ${D}${systemd_unitdir}/system

	#install -d ${D}/etc/
	#install ${S}/Launcher.conf ${D}/etc/
}
