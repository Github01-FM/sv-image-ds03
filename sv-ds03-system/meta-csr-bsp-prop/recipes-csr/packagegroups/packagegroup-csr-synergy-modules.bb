LICENSE = "MIT"
DESCRIPTION = "The synergy kernel modules packages"

PACKAGE_ARCH = "${MACHINE_ARCH}"

inherit packagegroup

RDEPENDS_${PN} = " kernel-module-csr-sdio-mmc kernel-module-hydra-skeleton kernel-module-sdio-lib-bt-a \
		kernel-module-hydra-core kernel-module-hydra-sdio kernel-module-oska"