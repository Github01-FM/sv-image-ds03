LICENSE = "Proprietary"
DESCRIPTION = "The ivi packages for csr solution"

PACKAGE_ARCH = "${MACHINE_ARCH}"

inherit packagegroup

RDEPENDS_${PN} = " csrpm equalizer kalimba-lib"
