require gstreamer1.0-plugins.inc

LICENSE = "LGPLv2+"

DEPENDS += "freetype liboil util-linux libvorbis"

inherit gettext

PACKAGES_DYNAMIC =+ "^libgst.*"

PACKAGECONFIG ??= " \
    ${@bb.utils.contains('DISTRO_FEATURES', 'alsa', 'alsa', '', d)} \
    orc ogg vorbis \
    "
PACKAGE_ARCH = "${MACHINE_ARCH}"
COMPATIBLE_MACHINE = "(atlas)"

EXTRA_SPECIFIC_CONF = " \
    --disable-adder \
    --disable-audiotestsrc \
    --disable-encoding \
    --enable-videotestsrc \
    --enable-videorate \
    --disable-videoscale \
    --enable-volume \
    --disable-ivorbis \
    --disable-theora \
    --disable-x \
    --disable-xvideo \
    --disable-pango \
    --enable-vorbis \
    "

# cdparanoia and libvisual do not seem to exist anywhere in OE
EXTRA_OECONF += " \
    --disable-freetypetest \
    --disable-cdparanoia \
    --disable-libvisual \
    ${GSTREAMER_1_0_ORC} \
    ${EXTRA_SPECIFIC_CONF} \
"

FILES_${PN} += "${datadir}/gst-plugins-base"

CACHED_CONFIGUREVARS_append_i586 = " ac_cv_header_emmintrin_h=no ac_cv_header_xmmintrin_h=no"
