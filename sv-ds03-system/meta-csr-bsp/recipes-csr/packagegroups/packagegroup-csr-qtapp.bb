LICENSE = "Proprietary"
DESCRIPTION = "The qt application packages for sirf solution"

PACKAGE_ARCH = "${MACHINE_ARCH}"

inherit packagegroup

RDEPENDS_${PN} = " csrlauncher"

