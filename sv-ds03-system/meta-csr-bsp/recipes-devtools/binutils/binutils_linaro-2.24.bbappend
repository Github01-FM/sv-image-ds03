SRC_URI = "\
     http://releases.linaro.org/archive/${MMYY}/components/toolchain/binutils-linaro/binutils-linaro-${BPV}-${RELEASE}.tar.xz \
     file://binutils-uclibc-100-uclibc-conf.patch \
     file://binutils-uclibc-300-001_ld_makefile_patch.patch \
     file://binutils-uclibc-300-006_better_file_error.patch \
     file://binutils-uclibc-300-012_check_ldrunpath_length.patch \
     file://binutils-uclibc-gas-needs-libm.patch \
     file://libtool-2.4-update.patch \
     file://libiberty_path_fix.patch \
     file://binutils-poison.patch \
     file://libtool-rpath-fix.patch \
     file://binutils-armv5e.patch \
     file://mips64-default-ld-emulation.patch \
     file://binutils-xlp-support.patch \
     file://fix-pr15815.patch;apply=no \
     file://fix-pr2404.patch \
     file://fix-pr16476.patch \
     file://fix-pr16428.patch \
     file://replace_macros_with_static_inline.patch;apply=no \
     file://0001-Fix-MMIX-build-breakage-from-bfd_set_section_vma-cha.patch;apply=no \
     file://binutils-uninitialised-warning.patch \
     file://0001-AArch64-Define-LP64-LE-loader-name.patch \
     file://0001-AArch64-Define-LP64-BE-linker-name.patch \
     "
