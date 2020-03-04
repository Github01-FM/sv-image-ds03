EXTRA_OECONF += " --with-inputdrivers=linuxinput,keyboard,tslib "

PACKAGECONFIG[linux-fusion] = "--enable-multi,--disable-multi,linux-fusion"

FILESEXTRAPATHS_prepend := "${THISDIR}/core:"

SRC_URI += "file://directfbrc file://g2d.patch file://0002_update_g2d_format.patch"

do_install_append () {
	install -d ${D}/etc/
	install ${WORKDIR}/directfbrc ${D}/etc/
}

FILES_${PN}-dev += "${libdir}/directfb-${RV}/gfxdrivers/*.la"

FILES_${PN} += "${libdir}/directfb-${RV}/gfxdrivers/*.so "
