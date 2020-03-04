FILESEXTRAPATHS_prepend := "${THISDIR}/openssh:"

SRC_URI += "file://sshdgenkeys.service"
SRC_URI += "file://sshdgenkeys.sh"

do_install_append () {
	# for read-only rootfs
	install -m 0755 ${WORKDIR}/sshdgenkeys.sh ${D}${bindir}/sshdgenkeys.sh
	sed -i -e 's/#HostKey \/etc\/ssh\/ssh_host_rsa_key/HostKey \/var\/lib\/ssh\/ssh_host_rsa_key/' ${D}${sysconfdir}/ssh/sshd_config
	sed -i -e 's/#HostKey \/etc\/ssh\/ssh_host_dsa_key/HostKey \/var\/lib\/ssh\/ssh_host_dsa_key/' ${D}${sysconfdir}/ssh/sshd_config
	sed -i -e '/HostKey \/var\/lib\/ssh\/ssh_host_dsa_key/a HostKey \/var\/lib\/ssh\/ssh_host_ecdsa_key' ${D}${sysconfdir}/ssh/sshd_config
}

FILES_${PN}-keygen += "${bindir}/sshdgenkeys.sh"
