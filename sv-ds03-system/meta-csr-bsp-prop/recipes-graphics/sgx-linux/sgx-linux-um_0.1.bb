#Copyright (C) 2014 CSR 
#if BR2_PACKAGE_CSR_GFX_BUILD_UM = "y"
DESCRIPTION = "csr gfx for sirf atlas"

PACKAGE_ARCH = "${MACHINE_ARCH}"
COMPATIBLE_MACHINE = "(atlas)"

#LICENSE = "Strictly Confidential"
LICENSE = "COMMERCIAL LICENSE"
LIC_FILES_CHKSUM = "file://${WORKDIR}/COPYING;md5=8cb934f218fc36bb0198d027a1825bf3"

require csr-gfx.inc

PROVIDES = "csr-gfx-um"
DEPENDS = "virtual/kernel"

SRCBRANCH ?= "a7_linux_wayland"
#SRCBRANCH_atlas7-arm = "1.12_RTM_3052601"
#SRCBRANCH_atlas6-arm = "1.11ED2580795"

FILESEXTRAPATHS_prepend := "${SRC_URI_csr-gfx-um-path}:"
SRC_URI_csr-gfx-um ?= ""

SRC_URI = "${SRC_URI_csr-gfx-um}"
SRC_URI += "file://COPYING"
SRC_URI += "file://egl.pc \
	file://glesv2.pc \
	file://wayland-egl.pc \
	file://rc.pvr \
"
SRCREV = "${AUTOREV}"

python () {
	if d.getVar('SRC_URI_csr-gfx-um-path',True) :
		d.setVar('S', "${WORKDIR}/csr-gfx-um")
	else:
		d.setVar('S', "${WORKDIR}/git") 
}

EXTRA_OEMAKE = 'CROSS_COMPILE=${TARGET_PREFIX} ARCH=arm'

INSANE_SKIP_${PN} += "ldflags already-stripped"

do_configure () {
	echo "CSR_GFX_KM_MAKE_OPTS is ${CSR_GFX_KM_MAKE_OPTS}"
	echo "CSR_GFX_UM_MAKE_OPTS is ${CSR_GFX_UM_MAKE_OPTS}"
}

do_compile () {
	oe_runmake ${CSR_GFX_UM_MAKE_OPTS} -C ${S}/${CSR_GFX_MK_PATH_UM} CSR_GFX_MK_PATH_UM=${CSR_GFX_MK_PATH_UM}   V=1
}

do_install () {
	install -d ${D}/usr/lib/
	for i in ${CSR_GFX_LIBS} ; do	
		install -D -m 0644 ${B}/${CSR_GFX_OUT_PATH_UM}/$i.so ${D}/usr/lib/
	done	
	install -d ${D}/usr/bin/
	for i in ${CSR_GFX_BIN} ; do	
		install -D -m 0644 ${B}/${CSR_GFX_OUT_PATH_UM}/$i ${D}/usr/bin/
	done
	for incdir in ${CSR_GFX_INCLUDE_DIRS} ;
	do
		install -d ${D}/usr/include/${incdir}
		install -D -m 0644 ${S}/eurasiacon/unittests/include/${incdir}/*.h ${D}/usr/include/${incdir}
	done

	install -D -m 0644 ${WORKDIR}/egl.pc ${D}/usr/lib/pkgconfig/egl.pc
	install -D -m 0644 ${WORKDIR}/glesv2.pc ${D}/usr/lib/pkgconfig/glesv2.pc
	install -D -m 0644 ${WORKDIR}/wayland-egl.pc ${D}/usr/lib/pkgconfig/wayland-egl.pc
	install -D -m 0755 ${WORKDIR}/rc.pvr ${D}/etc/init.d/S51rc.pvr

}
#end
