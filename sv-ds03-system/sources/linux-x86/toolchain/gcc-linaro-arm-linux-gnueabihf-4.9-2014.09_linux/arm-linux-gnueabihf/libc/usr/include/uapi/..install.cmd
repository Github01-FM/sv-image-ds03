cmd_/home/gz09/yocto/poky/sources/linux-x86/toolchain/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/arm-linux-gnueabihf/libc/usr//include/uapi/.install := /bin/sh scripts/headers_install.sh /home/gz09/yocto/poky/sources/linux-x86/toolchain/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/arm-linux-gnueabihf/libc/usr//include/uapi ./include/uapi ; /bin/sh scripts/headers_install.sh /home/gz09/yocto/poky/sources/linux-x86/toolchain/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/arm-linux-gnueabihf/libc/usr//include/uapi ./include ; /bin/sh scripts/headers_install.sh /home/gz09/yocto/poky/sources/linux-x86/toolchain/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/arm-linux-gnueabihf/libc/usr//include/uapi ./include/generated/uapi ; for F in ; do echo "\#include <asm-generic/$$F>" > /home/gz09/yocto/poky/sources/linux-x86/toolchain/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/arm-linux-gnueabihf/libc/usr//include/uapi/$$F; done; touch /home/gz09/yocto/poky/sources/linux-x86/toolchain/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/arm-linux-gnueabihf/libc/usr//include/uapi/.install
