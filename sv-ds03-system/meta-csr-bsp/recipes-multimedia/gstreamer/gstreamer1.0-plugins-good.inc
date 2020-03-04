require gstreamer1.0-plugins.inc

LICENSE = "LGPLv2.1+"

# libid3tag
DEPENDS += "gstreamer1.0-plugins-base zlib bzip2 libcap libsoup-2.4 libxml2 pulseaudio"

inherit gettext

PACKAGE_ARCH = "${MACHINE_ARCH}"
COMPATIBLE_MACHINE = "(atlas)"

PACKAGECONFIG ??= " \
    ${@bb.utils.contains('DISTRO_FEATURES', 'pulseaudio', 'pulseaudio', '', d)} \
    orc soup \
    "

EXTRA_SPECIFIC_CONF = " \
    --disable-alpha \
    --disable-audiofx \
    --disable-cutter \
    --disable-debugutils \
    --disable-deinterlace \
    --disable-dtmf \
    --disable-effectv \
    --disable-equalizer \
    --disable-flx \
    --disable-flac \
    --disable-goom \
    --disable-goom2k1 \
    --disable-icydemux \
    --disable-imagefreeze \
    --disable-interleave \
    --disable-law \
    --disable-level \
    --disable-multifile \
    --disable-multipart \
    --enable-pulse \
    --disable-replaygain \
    --disable-shapewipe \
    --disable-smpte \
    --disable-spectrum \
    --disable-videobox \
    --disable-videocrop \
    --disable-videofilter \
    --disable-videomixer \
    --disable-wavenc \
    --disable-y4m \
    --disable-cairo \
    --disable-gdk_pixbuf \
    --disable-jpeg \
    --disable-oss \
    --disable-libpng \
    --disable-speex \
    --disable-taglib \
    --disable-x \
    "
    

# the 1394 plugins require both libraw1394 and libiec61883
# the former is included in meta-oe, the latter isn't
# -> disabled

EXTRA_OECONF += " \
    --enable-oss \
    --enable-gst_v4l2 \
    --without-libv4l2 \
    --disable-directsound \
    --disable-waveform \
    --disable-oss4 \
    --disable-sunaudio \
    --disable-osx_audio \
    --disable-osx_video \
    --disable-aalib \
    --disable-libcaca \
    --disable-libdv \
    --disable-shout2 \
    --disable-examples \
    --disable-dv1394 \
    ${GSTREAMER_1_0_ORC} \
    ${EXTRA_SPECIFIC_CONF} \
"

