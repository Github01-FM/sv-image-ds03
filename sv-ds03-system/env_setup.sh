#!/bin/bash
# Created  by Xie Qihuai 2015-05-19    v0.1
# Modified by Xie Qihuai 2015-10-23    v0.2

function gettop
{
    local TOPFILE=build/setup_env.sh
    if [ -n "$TOP" -a -f "$TOP/$TOPFILE" ] ; then
        echo $TOP
    else
        if [ -f $TOPFILE ] ; then
            # The following circumlocution (repeated below as well) ensures
            # that we record the true directory name and not one that is
            # faked up with symlink names.
            PWD= /bin/pwd
        else
            # We redirect cd to /dev/null in case it's aliased to
            # a command that prints something as a side-effect
            # (like pushd)
            local HERE=$PWD
            T=
            while [ \( ! \( -f $TOPFILE \) \) -a \( $PWD != "/" \) ]; do
                cd .. > /dev/null
                T=`PWD= /bin/pwd`
            done
            cd $HERE > /dev/null
            if [ -f "$T/$TOPFILE" ]; then
                echo $T
            fi
        fi
    fi
}

export TOP=$(gettop)
if [ "$TOP" = "" ];then
    echo -e "\nError:"
    echo -e "TOP is empty."
    return 1;
fi

cd $TOP
if [ -d $TOP/build/conf ];then
    source ./oe-init-build-env
else
    TEMPLATECONF=meta-csr-bsp/conf source ./oe-init-build-env build
fi

export PATH=$TOP/sources/linux-x86/toolchain/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/bin:$PATH
export CROSS_COMPILE=arm-linux-gnueabihf-
export ARCH=arm
export PS1="\[\e[32;1m\][ds03]\[\e[0m\]:\w> "

####################################################################################
# Note: we modify the freertos build path
# 1. when you want to build csr's freertos need set
#     export FREERTOS_BUILD_PATH = /sources/freertos/FreeRTOS/Demo/CORTEX_M3_CSRatlas7_GCC
# 2. when you want to build Desay sv's freertos need set
#     export FREERTOS_BUILD_PATH=/sources/freertos/FreeRTOS/sv_m3/
####################################################################################
# EVB default
#export FREERTOS_BUILD_PATH=sources/freertos/FreeRTOS/Demo/CORTEX_M3_CSRatlas7_GCC
#KERNEL_CONFIG=prima2_defconfig
#DTB=atlas7-evb.dtb

# DS03 platform
export FREERTOS_BUILD_PATH=sources/freertos/FreeRTOS/sv_m3
KERNEL_CONFIG=ds03_defconfig
DTB=atlas7-ds03.dtb

export FREERTOS_DEP_bin=$(gettop)/${FREERTOS_BUILD_PATH}/gcc/freertos-dep.bin
export FREERTOS_IND_bin=$(gettop)/${FREERTOS_BUILD_PATH}/gcc/freertos-ind.bin
export FREERTOS_IND_BAK_bin=$(gettop)/${FREERTOS_BUILD_PATH}/gcc/freertos-ind-bak.bin
export FREERTOS_IND_A7BOOT_bin=$(gettop)/${FREERTOS_BUILD_PATH}/gcc/freertos-ind-a7boot.bin
export M3_TEST_ENABLE=no  # value: no or yes

export BOARD="atlas7cb"
export FREERTOS_TARGET="all"
function make-kernel ()
{
    local T=`pwd`
    S=$(gettop)/sources/kernel
    OUT=$(gettop)/build/kernel-OUT
    FIRMWARE=$(gettop)/build/tmp/sysroots/atlas7-arm/usr/lib/kalimba.fw.ihex

    # Configure
    if [ x`grep "CONFIG_ATLAS7DA_NOC=y" ${S}/arch/arm/configs/${KERNEL_CONFIG}` = x ]; then
        echo "CONFIG_ATLAS7DA_NOC=y" >> ${S}/arch/arm/configs/${KERNEL_CONFIG}
    fi
    if [ x`grep "CONFIG_CMA_SIZE_MBYTES" ${S}/arch/arm/configs/${KERNEL_CONFIG}` = x ]; then
        echo "CONFIG_CMA_SIZE_MBYTES=64" >> ${S}/arch/arm/configs/${KERNEL_CONFIG}
    fi
    cd ${S}
    make ${KERNEL_CONFIG} O=${OUT}

    # Update kalimba firmware which built by bitbake
    mkdir -p ${OUT}/firmware/kalimba
    cp $FIRMWARE ${OUT}/firmware/kalimba/

    # Compile
    make zImage O=${OUT} -j4
    if [ $? != 0 ];then
        echo -e "\nBuild kernel zImage error !!!"
        cd ${T}
        return 1
    fi

    make modules O=${OUT} -j4
    if [ $? != 0 ];then
        echo -e "\nBuild kernel module error !!!"
        cd ${T}
        return 1
    fi

    # Build dtb
    make ${DTB} O=${OUT}
    if [ $? != 0 ];then
        echo -e "\nBuild ${DTB} error !!!"
        cd ${T}
        return 1
    fi
        cd ${T}

}

function make-kernel-image
{
         local T=`pwd`
        make-kernel
        cd $(gettop)/image-sv
       ./gen-kernel-image.sh
        cd ${T}
}
function make-kernel-clean ()
{
    local T=`pwd`
    S=$(gettop)/sources/kernel
    OUT=$(gettop)/build/kernel-OUT

    cd ${S}
    make mrproper O=${OUT}
    cd ${T}
}

function make-ds03-image ()
{
    local T=`pwd`
    make-kernel
    bitbake u-boot
    cd $(gettop)/image-sv
    ./gen-sv-image.sh
    cd ${T}
}
function make-modules ()
{
    local T=`pwd`
    S=$(gettop)/sources/kernel
    OUT=$(gettop)/build/kernel-OUT

    cd ${S}
    make modules O=${OUT} -j4
    make modules_install O=${OUT} INSTALL_MOD_PATH=${OUT}
    cd ${T}
}

function make-dtb ()
{
    local T=`pwd`
    S=$(gettop)/sources/kernel
    OUT=$(gettop)/build/kernel-OUT

    cd ${S}
    # Build dtb
    make ${DTB} O=${OUT}
    if [ $? != 0 ];then
        echo -e "\nBuild ${DTB} error !!!"
        cd ${T}
        return 1
    fi
    cd ${T}
}

function make-menuconfig ()
{
    local T=`pwd`
    S=$(gettop)/sources/kernel
    OUT=$(gettop)/build/kernel-OUT

    cd ${S}
    make ${KERNEL_CONFIG} O=${OUT}

    # Menuconfig
    make menuconfig O=${OUT}
    cp ${OUT}/.config ${S}/arch/arm/configs/${KERNEL_CONFIG}

    cd ${T}
}


function ds03-tuxera-file-systems ()
{
#    KERNELDIR=${TOP}/build/kernel-OUT
    KERNELDIR=${TOP}/build/tmp/work/atlas7_arm-poky-linux-gnueabi/linux-kernel/3.18-r0/kernel
    if [ ! -d ${KERNELDIR} ];then
        echo -e "\n${KERNELDIR} Directory not exist."
        echo -e "bitbake kernel first."
        return 1
    fi

    local T=`pwd`
    echo "Update tuxera fat32"
    cd ${TOP}/sources/svauto-framework/tuxera-file-systems
    ./tuxera_update.sh  -a --target target/desay.d/csr-a7 --output-dir=${KERNELDIR} --user desay
    rm -rf kheaders*.tar.bz2
    STAMP=`date "+%Y%m%d%H%M%S"`
    echo "Date: ${STAMP}" > ${TOP}/sources/svauto-framework/tuxera-file-systems/stamp
    cd ${T}
    echo -e "Done !!!\n"
}

