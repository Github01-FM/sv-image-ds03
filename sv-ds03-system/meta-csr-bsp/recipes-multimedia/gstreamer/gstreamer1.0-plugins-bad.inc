require gstreamer1.0-plugins.inc

LICENSE = "LGPLv2+ & LGPLv2.1+ "

DEPENDS += "gstreamer1.0-plugins-base bzip2 libpng jpeg mpg123 virtual/kernel"

PACKAGE_ARCH = "${MACHINE_ARCH}"
COMPATIBLE_MACHINE = "(atlas)"

inherit gettext

# these plugins have not been ported to 1.0 (yet):
#   directdraw vcd apexsink dc1394 lv2 linsys musepack mythtv
#   nas timidity teletextdec sdl xvid wininet acm gsettings
#   sndio qtwrapper cdxaparse dccp faceoverlay hdvparse tta
#   mve nuvdemux osx_video patchdetect quicktime real sdi
#   videomeasure gsettings

# these plugins have no corresponding library in OE-core or meta-openembedded:
#   openni2 winks direct3d directdraw directsound winscreencap osx_video
#   apple_media android_media avc chromaprint daala dts gme gsm kate ladspa mimic
#   mpeg2enc mplex ofa openjpeg opensles pvr resindvd rtmp soundtouch spandsp spc
#   srtp vdpau voaacenc voamrwbenc wasapi zbar

PACKAGECONFIG ??= " \
    orc \
    ${@bb.utils.contains('DISTRO_FEATURES', 'wayland', 'wayland', '', d)} \
    "

EXTRA_SPECIFIC_CONF = " \
    --disable-accurip \
    --disable-adpcmdec \
    --disable-adpcmenc \
    --disable-aiff \
    --disable-audiofxbad \
    --disable-audiomixer \
    --disable-audiovisualizers \
    --disable-autoconvert \
    --disable-bayer \
    --disable-camerabin2 \
    --disable-coloreffects \
    --disable-compositor \
    --disable-dataurisrc \
    --disable-debugutils \
    --disable-dvbsuboverlay \
    --disable-dvb \
    --disable-dvdspu \
    --disable-festival \
    --disable-fieldanalysis \
    --disable-freeverb \
    --disable-frei0r \
    --disable-gaudieffects \
    --disable-gdp \
    --disable-geometrictransform \
    --disable-inter \
    --disable-interlace \
    --disable-ivfparse \
    --disable-ivtc \
    --disable-jp2kdecimator \
    --disable-jpegformat \
    --disable-librfb \
    --disable-liveadder \
    --disable-midi \
    --disable-mpegpsmux \
    --disable-mpegtsmux \
    --disable-mxf \
    --disable-pcapparse \
    --disable-pnm \
    --disable-removesilence \
    --disable-sdp \
    --disable-segmentclip \
    --disable-siren \
    --disable-smooth \
    --disable-speed \
    --disable-subenc \
    --disable-videofilters \
    --disable-videosignal \
    --disable-vmnc \
    --disable-y4m \
    --disable-yadif \
    --disable-bz2 \
    --disable-bluez \
    --disable-curl \
    --disable-dash \
    --disable-decklink \
    --disable-directfb\
    --disable-fbdev \
    --disable-hls \
    --disable-neon \
    --disable-sbc \
    --disable-sndfile \
    --disable-shm \
    --disable-smoothstreaming \
    --enable-mpg123 \
    "

EXTRA_OECONF += " \
    --enable-dvb \
    --enable-shm \
    --enable-fbdev \
    --enable-decklink \
    --disable-acm \
    --disable-android_media \
    --disable-apexsink \
    --disable-apple_media \
    --disable-avc \
    --disable-chromaprint \
    --disable-daala \
    --disable-dc1394 \
    --disable-direct3d \
    --disable-directdraw \
    --disable-directsound \
    --disable-dts \
    --disable-egl \
    --disable-gles2 \
    --disable-gme \
    --disable-gsettings \
    --disable-gsm \
    --disable-kate \
    --disable-ladspa \
    --disable-linsys \
    --disable-lv2 \
    --disable-mimic \
    --disable-mpeg2enc \
    --disable-mplex \
    --disable-musepack \
    --disable-mythtv \
    --disable-nas \
    --disable-ofa \
    --disable-opengl \
    --disable-openjpeg \
    --disable-opensles \
    --disable-osx_video \
    --disable-pvr \
    --disable-quicktime \
    --disable-resindvd \
    --disable-sdl \
    --disable-sdltest \
    --disable-sndio \
    --disable-soundtouch \
    --disable-spandsp \
    --disable-spc \
    --disable-srtp \
    --disable-teletextdec \
    --disable-timidity \
    --disable-uvch264 \
    --disable-vcd \
    --disable-vdpau \
    --disable-voaacenc \
    --disable-voamrwbenc \
    --disable-wasapi \
    --disable-wildmidi \
    --disable-wininet \
    --disable-winscreencap \
    --disable-xvid \
    --disable-zbar \
    ${GSTREAMER_1_0_ORC} \
    ${EXTRA_SPECIFIC_CONF} \
    "
PACKAGECONFIG[wayland] = "--enable-wayland,--disable-wayland,wayland"

ARM_INSTRUCTION_SET = "arm"

FILES_gstreamer1.0-plugins-bad-opencv += "${datadir}/gst-plugins-bad/1.0/opencv*"

