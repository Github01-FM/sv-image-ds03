SUMMARY = "Enhances systemd compatilibity with existing SysVinit scripts"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/LICENSE;md5=4d92cd373abda3937c2bc47fbc49d690"

PR = "r29"

DEPENDS = "systemd-systemctl-native"

inherit allarch

ALLOW_EMPTY_${PN} = "1"

SYSTEMD_DISABLED_SYSV_SERVICES = " \
  busybox-udhcpc \
  nfsserver \
  nfscommon \
  syslog.busybox \
  ldconfig \
"

SYSTEMD_DISABLED_SERVICES = " \
  systemd-journald \
  systemd-journald-dev-log \
  systemd-journal-flush \
  systemd-udev-settle \
  systemd-udev-trigger \
"

pkg_postinst_${PN} () {
	cd $D${sysconfdir}/init.d

	echo "Disabling the following sysv scripts: "

	OPTS=""

	if [ -n "$D" ]; then
		OPTS="--root=$D"
	fi

	for i in ${SYSTEMD_DISABLED_SYSV_SERVICES} ; do
		if [ \( -e $i -o $i.sh \) -a ! \( -e $D${sysconfdir}/systemd/system/$i.service -o  -e $D${systemd_unitdir}/system/$i.service \) ] ; then
			echo -n "$i: " ; systemctl ${OPTS} mask $i.service
		fi
	done ; echo

	for i in ${SYSTEMD_DISABLED_SERVICES} ; do
		echo -n "$i: " ; systemctl ${OPTS}  mask $i.service
	done; echo
}

RDPEPENDS_${PN} = "systemd"
