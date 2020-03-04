SUMMARY = "System and service manager for Linux, replacing SysVinit"

FILESEXTRAPATHS_prepend := "${THISDIR}/systemd:"
SRC_URI += "file://0001-udev-use-other-log-api-to-fix-build-error.patch"
SRC_URI += "file://0001-modify-for-udevd-to-make-user-can-use-mount-dev.patch"
SRC_URI += "file://0001-systemd-remove-unneed-getty-tty1.service-for-csr-bsp.patch"
SRC_URI += "file://0001-udev-make-udev-support-mt-touch-id-support.patch"
SRC_URI += "file://remnt.sh"
SRC_URI += "file://mntvar.sh"
SRC_URI += "file://mountvar.service"
SRC_URI += "file://0001-journal-reduce-minimum-journal-file-size-to-512-KiB.patch"

do_install_append(){
	install -m 0755 ${WORKDIR}/mntvar.sh ${D}${bindir}/mntvar.sh
	install -m 0755 ${WORKDIR}/remnt.sh ${D}${bindir}/remnt.sh
	install -m 0644 ${WORKDIR}/mountvar.service ${D}${systemd_unitdir}/system
	ln -snf ../usr/lib/os-release ${D}${sysconfdir}/os-release
	ln -snf ../usr/share/zoneinfo/UTC ${D}${sysconfdir}/localtime
	ln -sf ../systemd-readahead-collect.service ${D}${systemd_unitdir}/system/multi-user.target.wants/systemd-readahead-collect.service
	ln -sf ../systemd-readahead-replay.service ${D}${systemd_unitdir}/system/multi-user.target.wants/systemd-readahead-replay.service
	ln -sf ../mountvar.service ${D}${systemd_unitdir}/system/sysinit.target.wants/mountvar.service
	sed -i '/PrivateTmp/d' ${D}${systemd_unitdir}/system/systemd-timesyncd.service
	sed -i -e '/ConditionPathExists=!\/run\/systemd\/readahead\/done/a ConditionPathExists=!\/\.readahead' ${D}${systemd_unitdir}/system/systemd-readahead-collect.service
	sed -i -e '/Type=notify/a ExecStartPre=\/bin\/sh \/usr\/bin\/remnt.sh' ${D}${systemd_unitdir}/system/systemd-readahead-collect.service
	sed -i -e 's/.*RuntimeMaxUse.*/RuntimeMaxUse=1M/' ${D}${sysconfdir}/systemd/journald.conf
	sed -i -e 's/.*RuntimeMaxFileSize.*/RuntimeMaxFileSize=512K/' ${D}${sysconfdir}/systemd/journald.conf
	rm -rf ${D}${systemd_unitdir}/system/sockets.target.wants/systemd-journald.socket
	rm -rf ${D}${systemd_unitdir}/system/sockets.target.wants/systemd-journald-dev-log.socket
}

FILES_${PN} += "usr/bin/\
		usr/bin/mntvar.sh\
		etc/localtime\
		etc/os-release"
