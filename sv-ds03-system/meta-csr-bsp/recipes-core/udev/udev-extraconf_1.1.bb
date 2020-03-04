SUMMARY = "Extra machine specific configuration files"
DESCRIPTION = "Extra machine specific configuration files for udev, specifically blacklist information."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/LICENSE;md5=4d92cd373abda3937c2bc47fbc49d690 \
                    file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

PACKAGE_ARCH = "${MACHINE_ARCH}"
COMPATIBLE_MACHINE = "(atlas)"

SRC_URI = " \
       file://automount.rules \
       file://mount.sh \
       file://mount.blacklist \
       file://autonet.rules \
       file://network.sh \
       file://localextra.rules \
       file://usb-dev.sh \
"

SRC_URI_append_atlas7-arm = " \
       file://70-persistent-net.rules \
       file://85-usb.rules \
       file://86-misc.rules \
       file://87-video.rules \
       file://extcon.sh \
"

SRC_URI_append_atlas7-arm = " \
       file://sp.sh \
       file://usbevents.sh \
       file://usbhub.sh \
"

do_install() {
    install -d ${D}${sysconfdir}/udev/rules.d

    install -m 0644 ${WORKDIR}/automount.rules     ${D}${sysconfdir}/udev/rules.d/automount.rules
    install -m 0644 ${WORKDIR}/autonet.rules       ${D}${sysconfdir}/udev/rules.d/autonet.rules
    install -m 0644 ${WORKDIR}/localextra.rules    ${D}${sysconfdir}/udev/rules.d/localextra.rules

    install -m 0644 ${WORKDIR}/mount.blacklist     ${D}${sysconfdir}/udev/

    install -d ${D}${sysconfdir}/udev/scripts/

    install -m 0755 ${WORKDIR}/mount.sh ${D}${sysconfdir}/udev/scripts/mount.sh
    install -m 0755 ${WORKDIR}/network.sh ${D}${sysconfdir}/udev/scripts
    install -m 0644 ${WORKDIR}/usb-dev.sh ${D}${sysconfdir}/udev/scripts/usb-dev.sh
}

do_install_prepend () {
	if [ -e "${WORKDIR}/70-persistent-net.rules" ]; then
		install -d ${D}${sysconfdir}/udev/rules.d
		install -m 0644 ${WORKDIR}/70-persistent-net.rules ${D}${sysconfdir}/udev/rules.d
	fi
	if [ -e "${WORKDIR}/85-usb.rules" ]; then
		install -d ${D}${sysconfdir}/udev/rules.d
		install -m 0644 ${WORKDIR}/85-usb.rules ${D}${sysconfdir}/udev/rules.d
	fi

	if [ -e "${WORKDIR}/86-misc.rules" ]; then
		install -d ${D}${sysconfdir}/udev/rules.d
		install -m 0644 ${WORKDIR}/86-misc.rules ${D}${sysconfdir}/udev/rules.d
	fi

	if [ -e "${WORKDIR}/87-video.rules" ]; then
		install -d ${D}${sysconfdir}/udev/rules.d
		install -m 0644 ${WORKDIR}/87-video.rules ${D}${sysconfdir}/udev/rules.d
	fi

	if [ -e "${WORKDIR}/extcon.sh" ]; then
		install -d ${D}${sysconfdir}/udev/scripts
		install -m 0755 ${WORKDIR}/extcon.sh ${D}${sysconfdir}/udev/scripts
	fi

	if [ -e "${WORKDIR}/sp.sh" ]; then
		install -d ${D}${sysconfdir}/modules
		install -m 0755 ${WORKDIR}/sp.sh ${D}${sysconfdir}/modules
	fi

	if [ -e "${WORKDIR}/usbevents.sh" ]; then
		install -d ${D}${sysconfdir}/modules
		install -m 0755 ${WORKDIR}/usbevents.sh ${D}${sysconfdir}/modules
	fi

	if [ -e "${WORKDIR}/usbhub.sh" ]; then
		install -d ${D}${sysconfdir}/modules
		install -m 0755 ${WORKDIR}/usbhub.sh ${D}${sysconfdir}/modules
	fi
}


FILES_${PN} = "${sysconfdir}"
RDEPENDS_${PN} = "udev"
CONFFILES_${PN} = "${sysconfdir}/udev/mount.blacklist"

# to replace udev-extra-rules from meta-oe
RPROVIDES_${PN} = "udev-extra-rules"
RREPLACES_${PN} = "udev-extra-rules"
RCONFLICTS_${PN} = "udev-extra-rules"
