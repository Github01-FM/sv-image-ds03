cmd_/home/gz09/yocto/poky/sources/linux-x86/toolchain/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/arm-linux-gnueabihf/libc/usr//include/linux/netfilter/ipset/.install := /bin/sh scripts/headers_install.sh /home/gz09/yocto/poky/sources/linux-x86/toolchain/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/arm-linux-gnueabihf/libc/usr//include/linux/netfilter/ipset ./include/uapi/linux/netfilter/ipset ip_set.h ip_set_bitmap.h ip_set_hash.h ip_set_list.h; /bin/sh scripts/headers_install.sh /home/gz09/yocto/poky/sources/linux-x86/toolchain/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/arm-linux-gnueabihf/libc/usr//include/linux/netfilter/ipset ./include/linux/netfilter/ipset ; /bin/sh scripts/headers_install.sh /home/gz09/yocto/poky/sources/linux-x86/toolchain/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/arm-linux-gnueabihf/libc/usr//include/linux/netfilter/ipset ./include/generated/uapi/linux/netfilter/ipset ; for F in ; do echo "\#include <asm-generic/$$F>" > /home/gz09/yocto/poky/sources/linux-x86/toolchain/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/arm-linux-gnueabihf/libc/usr//include/linux/netfilter/ipset/$$F; done; touch /home/gz09/yocto/poky/sources/linux-x86/toolchain/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/arm-linux-gnueabihf/libc/usr//include/linux/netfilter/ipset/.install
