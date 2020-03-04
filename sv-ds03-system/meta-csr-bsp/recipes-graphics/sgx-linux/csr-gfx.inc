#if BR2_PACKAGE_CSR_GFX_DEBUG = "y"
#CSR_GFX_DEBUG_UM = "debug"
#CSR_GFX_DEBUG_KM = "debug"
#else
CSR_GFX_DEBUG_UM = "release"
CSR_GFX_DEBUG_KM = "release"
#endif

BR2_BOARD_atlas6-arm = "atlas6cb"
BR2_BOARD_atlas7-arm = "atlas7cb"
ARCH = "arm"

CSR_GFX_MK_PATH_UM = "eurasiacon/build/linux2/sirfsoc_linux"
CSR_GFX_MK_PATH_KM_1 = "eurasiacon/build/linux2/sirfsoc_linux"
#CSR_GFX_MK_PATH_KM = "km/eurasiacon/build/linux2/sirfsoc_linux"

CSR_GFX_OUT_PATH_UM = "eurasiacon/binary2_sirfsoc_linux_${CSR_GFX_DEBUG_UM}/target"
CSR_GFX_OUT_PATH_KM = "eurasiacon/binary2_sirfsoc_linux_${CSR_GFX_DEBUG_KM}/target"
#CSR_GFX_OUT_PATH_KM = "km/eurasiacon/binary2_sirfsoc_linux_${CSR_GFX_DEBUG_KM}/target"

CSR_GFX_BIN_PATH = "rel"

CSR_GFX_KM_MAKE_OPTS = "\
	${LINUX_MAKE_FLAGS} \
	BUILD=${CSR_GFX_DEBUG_KM} \
	BOARD=${BR2_BOARD} \
	KERNELDIR=${STAGING_KERNEL_DIR} \
"

CSR_GFX_UM_MAKE_OPTS = "\
	BUILD=${CSR_GFX_DEBUG_UM} \
	BOARD=${BR2_BOARD} \
	KERNELDIR=${STAGING_KERNEL_DIR} \
	CROSS_COMPILE=${TARGET_PREFIX} \
	SYSROOTDIR=${PKG_CONFIG_SYSROOT_DIR} \
"

#PLATFORM=Linux_armv7hf
CSR_GFX_DEMO_MAKE_OPTS = "\
	PLATFORM=Linux_armv7hf \
	X11BUILD=0 \
"

CSR_GFX_BIN = "pvrsrvctl"

#if BR2_PACKAGE_CSR_GFX_DEBUG
CSR_GFX_BIN += "\
	eglinfo \
	gles1test1 gles2test1 pvr2d_test services_test \
	sgx_blit_test sgx_clipblit_test sgx_flip_test sgx_init_test \
	sgx_render_flip_test \
"
#endif

CSR_GFX_MODS = "pvrsrvkm dc_sirfsocfb_linux"

CSR_GFX_LIBS = "libpvr2d libpvrEWS_WSEGL libpvrEWS_REMWSEGL libpvrPVR2D_BLITWSEGL \
	libpvrPVR2D_FLIPWSEGL libpvrPVR2D_FRONTWSEGL libpvrPVR2D_LINUXFBWSEGL \
	libPVRScopeServices libsrv_init libsrv_um libusc \
	libEGL libGLES_CM libGLESv2 libglslcompiler libIMGegl \
"
CSR_GFX_LIBS += " libpvrWAYLAND_WSEGL libwayland-egl "

#Get rid of Navigation Navigation3D 
CSR_GFX_DEMOS_ES2 = "DeferredShading Skybox2 MagicLantern Fractal Coverflow Navigation3D \
	Glass Shaders ChameleonMan FilmTV BinaryShader ExampleUI ParticleSystem \
	Water PhantomMask Navigation \
"
#Get rid of Navigation Navigation3D
CSR_GFX_DEMOS_ES = "Particles Fur Vase Fractal Coverflow Skybox Navigation3D \
	Mouse EvilSkull PolyBump FilmTV Navigation \
"
CSR_GFX_DEMOS_LOC = "Examples/Advanced"
CSR_GFX_DEMOS_MAKE_LOC = "Build/LinuxGeneric"
CSR_GFX_DEMOS_BIN_LOC = "Build/Linux_armv7hf/ReleaseNullWS"

CSR_GFX_INCLUDE_PATH = "${CSR_GFX_BIN_PATH}/include"
CSR_GFX_INCLUDE_DIRS = "EGL GLES GLES2 KHR"




