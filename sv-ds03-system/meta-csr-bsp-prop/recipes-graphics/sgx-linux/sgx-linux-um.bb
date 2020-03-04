#Copyright (C) 2014 CSR 
DESCRIPTION = "csr gfx prebuilt lib for csr "

PACKAGE_ARCH = "${MACHINE_ARCH}"
COMPATIBLE_MACHINE = "(atlas)"

LICENSE = "COMMERCIAL LICENSE"
LIC_FILES_CHKSUM = "file://${WORKDIR}/COPYING;md5=8cb934f218fc36bb0198d027a1825bf3"

require csr-gfx.inc

PROVIDES = "virtual/libgl virtual/libgles1 virtual/libgles2 virtual/egl"
DEPENDS = "virtual/kernel"

#SRCBRANCH ?= "1.12_RTM_3052601"
SRCBRANCH ?= "a7_linux_wayland"
SRCBRANCH_atlas7-arm = "1.12_RTM_3052601"
SRCBRANCH_atlas6-arm = "1.11ED2580795"

FILESEXTRAPATHS_prepend := "${SRC_URI_sgx-linux-um-path}:"
SRC_URI_sgx-linux-um ?= "git://${CSR_REPO_SITE}/git/linux/graphics/sgx-linux-rel-v531.git;protocol=http;branch=${SRCBRANCH}"

SRC_URI = "${SRC_URI_sgx-linux-um}"
SRC_URI += "file://COPYING"
SRC_URI += "file://egl.pc \
	file://glesv2.pc \
	file://wayland-egl.pc \
	file://rc.pvr \
	file://pvr.service \
"
SRCREV = "${AUTOREV}"

INSANE_SKIP_${PN} += "ldflags already-stripped"
#remove -Wl,--hash-style=gnu from it to avoid qa error for the prebuilt lib
LDFLAGS = "-Wl,-O1"

python () {
	if d.getVar('SRC_URI_sgx-linux-um-path',True) :
		d.setVar('S', "${WORKDIR}/sgx-linux-um")
	else:
		d.setVar('S', "${WORKDIR}/git") 
}

inherit systemd
SYSTEMD_SERVICE_${PN} = "pvr.service"

do_install () {
	for lib in ${CSR_GFX_LIBS} ;
	do	
		install -D -m 0644 ${S}/${lib}.so ${D}/usr/lib/${lib}.so;
	done
	for incdir in ${CSR_GFX_INCLUDE_DIRS} ;
	do
		install -d ${D}/usr/include/${incdir} 
		install -D -m 0644 ${S}/include/${incdir}/*.h ${D}/usr/include/${incdir}
	done

	install -D -m 0644 ${WORKDIR}/egl.pc ${D}/usr/lib/pkgconfig/egl.pc
	install -D -m 0644 ${WORKDIR}/glesv2.pc ${D}/usr/lib/pkgconfig/glesv2.pc
	install -D -m 0644 ${WORKDIR}/wayland-egl.pc ${D}/usr/lib/pkgconfig/wayland-egl.pc
	install -D -m 0755 ${WORKDIR}/rc.pvr ${D}/usr/bin/pvrsvr.sh
	install -D ${S}/pvrsrvctl ${D}/usr/bin/pvrsrvctl

	install -d ${D}${systemd_unitdir}/system
	install -c -m 0644 ${WORKDIR}/pvr.service ${D}${systemd_unitdir}/system
}

PACKAGES = "${PN} ${PN}-dbg ${PN}-staticdev ${PN}-dev ${PN}-doc ${PN}-locale ${PACKAGE_BEFORE_PN}"
FILES_${PN} += " ${libdir}/*.so"
