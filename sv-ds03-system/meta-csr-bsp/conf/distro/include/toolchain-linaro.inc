GCCVERSION ?= "linaro-4.9%"
BINUVERSION ?= "linaro-2.24%"
GLIBCVERSION ?= "2.20%"
GLIBCINITVERSION ?= "2.20%"
LINUXLIBCVERSION ?= "4.1"

PREFERRED_VERSION_gcc ?= "${GCCVERSION}"
PREFERRED_VERSION_binutils ?= "${BINUVERSION}"
PREFERRED_VERSION_linux-libc-headers ?= "${LINUXLIBCVERSION}"
PREFERRED_VERSION_glibc ?= "${GLIBCVERSION}"
PREFERRED_VERSION_glibc-locale ?= "${GLIBCVERSION}"
PREFERRED_VERSION_glibc-initial ?= "${GLIBCINITVERSION}"

# Select external binary prebuilt Linaro toolchain
TCMODE = "external-linaro"
TCLIBC = "external-linaro-toolchain"

ELT_TARGET_SYS = "arm-linux-gnueabihf"
TARGET_VENDOR = ""

PREFERRED_PROVIDER_virtual/${TARGET_PREFIX}gcc-intermediate = "external-linaro-toolchain"

# Set some vars to help locate the toolchain automatically from PATH
TOOLCHAIN_SYS := "${ELT_TARGET_SYS}"
TOOLCHAIN_PREFIX := "${TOOLCHAIN_SYS}-"

# Use specific libc headers instead of ones provided by toolchain
PREFERRED_PROVIDER_linux-libc-headers-dev_forcevariable = "linux-libc-headers"
PREFERRED_PROVIDER_virtual/linux-libc-headers_forcevariable = "linux-libc-headers"

PREFERRED_PROVIDER_gcc-cross-canadian-${TRANSLATED_TARGET_ARCH} ?= "external-linaro-sdk-toolchain"
PREFERRED_PROVIDER_binutils-cross-canadian-${TRANSLATED_TARGET_ARCH} ?= "external-linaro-sdk-toolchain"
#PREFERRED_PROVIDER_gdb-cross-canadian-${TRANSLATED_TARGET_ARCH} = "external-linaro-sdk-toolchain"

# Special case for gdb to be built as part of canadian-cross-sdk, instead of packaged from external toolchain
bindir_pn-gdb-cross-canadian-${TRANSLATED_TARGET_ARCH} = "${exec_prefix}/bin"
PREFERRED_PROVIDER_gdb-cross-canadian-${TRANSLATED_TARGET_ARCH} = "gdb-cross-canadian-${TRANSLATED_TARGET_ARCH}"

# Set up runtime preferences
PREFERRED_PROVIDER_libgcc = "external-linaro-toolchain"
PREFERRED_PROVIDER_gcc-runtime = "external-linaro-toolchain"

SDKGCCVERSION ?= "linaro-4.9%"
SDKBINUVERSION ?= "linaro-2.24%"
SDKGLIBCVERSION ?= "linaro-2.20%"
SDKLINUXLIBCVERSION ?= "4.1%"

PREFERRED_VERSION_gcc-crosssdk ?= "${SDKGCCVERSION}"
PREFERRED_VERSION_gcc-crosssdk-initial ?= "${SDKGCCVERSION}"
PREFERRED_VERSION_gcc-crosssdk-intermediate ?= "${SDKGCCVERSION}"
PREFERRED_VERSION_nativesdk-gcc-runtime ?= "${SDKGCCVERSION}"
PREFERRED_VERSION_nativesdk-libgcc ?= "${SDKGCCVERSION}"
PREFERRED_VERSION_binutils-crosssdk ?= "${SDKBINUVERSION}"
PREFERRED_VERSION_nativesdk-linux-libc-headers ?= "${SDKLINUXLIBCVERSION}"
PREFERRED_VERSION_nativesdk-glibc ?= "${SDKGLIBCVERSION}"
PREFERRED_VERSION_nativesdk-glibc-initial ?= "${SDKGLIBCVERSION}"
