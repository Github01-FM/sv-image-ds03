cmd_/home/gz09/yocto/poky/sources/linux-x86/toolchain/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/arm-linux-gnueabihf/libc/usr//include/scsi/.install := /bin/sh scripts/headers_install.sh /home/gz09/yocto/poky/sources/linux-x86/toolchain/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/arm-linux-gnueabihf/libc/usr//include/scsi ./include/uapi/scsi scsi_bsg_fc.h scsi_netlink.h scsi_netlink_fc.h; /bin/sh scripts/headers_install.sh /home/gz09/yocto/poky/sources/linux-x86/toolchain/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/arm-linux-gnueabihf/libc/usr//include/scsi ./include/scsi ; /bin/sh scripts/headers_install.sh /home/gz09/yocto/poky/sources/linux-x86/toolchain/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/arm-linux-gnueabihf/libc/usr//include/scsi ./include/generated/uapi/scsi ; for F in ; do echo "\#include <asm-generic/$$F>" > /home/gz09/yocto/poky/sources/linux-x86/toolchain/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/arm-linux-gnueabihf/libc/usr//include/scsi/$$F; done; touch /home/gz09/yocto/poky/sources/linux-x86/toolchain/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/arm-linux-gnueabihf/libc/usr//include/scsi/.install