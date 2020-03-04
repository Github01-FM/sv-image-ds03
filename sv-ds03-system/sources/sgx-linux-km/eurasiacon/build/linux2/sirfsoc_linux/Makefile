PVR_SYSTEM	:= sgx_sirfsoc
KERNEL_COMPONENTS := srvkm bufferclass_example

ifneq (,$(filter $(BOARD),prima2cb atlas7cb))
ifeq ($(SGXCORE),)
SGXCORE := 531
SGX_CORE_REV := 110
SUPPORT_SLC := 1
endif
endif

ifeq ($(BOARD), atlas6cb)
ifeq ($(SGXCORE),)
SGXCORE := 520
SGX_CORE_REV := 111
endif
endif

SUPPORT_ION := 1
SUPPORT_DC_CMDCOMPLETE_WHEN_NO_LONGER_DISPLAYED := 1

SUPPORT_LINUX_USING_WORKQUEUES := 1

DISPLAY_CONTROLLER := dc_sirfsocfb_linux
DISPLAY_CONTROLLER_COMPONENT := dc_sirfsocfb_linux

PVRSRV_USSE_EDM_STATUS_DEBUG=1
PVRSRV_DUMP_MK_TRACE=1
#USE_SUPPORT_NO_TA3D_OVERLAP=1

# Now SGX device is defined in device tree which causes platform_register_driver
# do probing. So the next platform_register_device is not necessary, or else this
# will do probing again which may cause PVRSRVDriverRemove is done twice if module
# is unloaded.
SPECIFY_SGX_IN_DT=1

include ../kernel_version.mk
# Only enable active power management if passive power management is
# enabled, as indicated by LDM_PLATFORM being set to 1.  On OMAP,
# the system can suspend in the case where active power management is
# enabled in the SGX driver, but passive power management isn't. As
# passive power management isn't enabled, the driver won't see the
# system suspend/resume events, and so won't take appropriate action.
LDM_PLATFORM ?= 1

include ../common/xorg_test.mk
ifeq ($(want_xorg),1)
SUPPORT_DRI_DRM := 1
OPK_DEFAULT := libpvrPVR2D_BLITWSEGL.so
OPK_FALLBACK := libpvrPVR2D_DRIWSEGL.so
else
OPK_DEFAULT := libpvrPVR2D_FLIPWSEGL.so
OPK_FALLBACK := libpvrPVR2D_BLITWSEGL.so
endif

ifeq ($(SUPPORT_DRI_DRM),1)
EXTRA_PVRSRVKM_COMPONENTS += $(DISPLAY_CONTROLLER_COMPONENT)
else
KERNEL_COMPONENTS += $(DISPLAY_CONTROLLER_COMPONENT)
endif

SUPPORT_SGX_UKERNEL_DCU_BYPASS := 1

# Should be last
include ../config/core.mk
include ../common/xorg.mk
include ../common/dridrm.mk
include ../common/opencl.mk

$(eval $(call TunableBothConfigC,SJB_24956_PATCH,))
# If set, services allows two flips to enter the processing queue,
# and does not add read dependencies to the set of buffers last
# flipped to.
$(eval $(call TunableKernelConfigC,SUPPORT_DC_CMDCOMPLETE_WHEN_NO_LONGER_DISPLAYED,))
