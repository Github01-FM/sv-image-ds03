DESCRIPTION = "csr gfx for csr atlas"

PACKAGE_ARCH = "${MACHINE_ARCH}"
COMPATIBLE_MACHINE = "(atlas)"

LICENSE = "MIT | GPL"
LIC_FILES_CHKSUM = "file://MIT-COPYING;md5=8c2810fa6bfdc5ae5c15a0c1ade34054 \
		    file://GPL-COPYING;md5=60422928ba677faaa13d6ab5f5baaa1e"

require csr-gfx.inc

DEPENDS = "virtual/kernel"

#SRCBRANCH ?= "1.12_RTM_3052601"
SRCBRANCH ?= "a7_linux_ion"
SRCBRANCH_atlas7-arm = "1.12_RTM_3052601"
SRCBRANCH_atlas6-arm = "1.11ED2580795"

FILESEXTRAPATHS_prepend := "${SRC_URI_sgx-linux-km-path}:"
SRC_URI_sgx-linux-km ?= "git://${CSR_REPO_SITE}/git/linux/graphics/sgx-linux-km.git;protocol=http;branch=${SRCBRANCH}"

SRC_URI = "${SRC_URI_sgx-linux-km}"
SRCREV = "${AUTOREV}"

python () {
	if d.getVar('SRC_URI_sgx-linux-km-path',True) :
		d.setVar('S', "${WORKDIR}/sgx-linux-km")
	else:
		d.setVar('S', "${WORKDIR}/git") 
}

inherit module

LINUX_VERSION_PROBED = "${KERNEL_VERSION}"

EXTRA_OEMAKE = 'CROSS_COMPILE=${TARGET_PREFIX} ARCH=arm'

do_configure () {
	echo "CSR_GFX_KM_MAKE_OPTS is ${CSR_GFX_KM_MAKE_OPTS}"
	echo "CSR_GFX_UM_MAKE_OPTS is ${CSR_GFX_UM_MAKE_OPTS}"
}

do_compile () {
	oe_runmake ${CSR_GFX_KM_MAKE_OPTS} -C ${S}/${CSR_GFX_MK_PATH_KM_1} CSR_GFX_MK_PATH_KM=${CSR_GFX_MK_PATH_KM_1}   V=1
}

do_install () {
	install -d ${D}/lib/modules/${LINUX_VERSION_PROBED}/extra/
	for i in ${CSR_GFX_MODS} ; do	
		install -D -m 0644 ${B}/${CSR_GFX_OUT_PATH_KM}/$i.ko ${D}/lib/modules/${LINUX_VERSION_PROBED}/extra/
	done	
}
	
