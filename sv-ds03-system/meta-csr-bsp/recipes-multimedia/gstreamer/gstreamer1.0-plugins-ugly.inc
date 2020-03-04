require gstreamer1.0-plugins.inc

LICENSE = "LGPLv2.1+ & LGPLv2+"

DEPENDS += "gstreamer1.0-plugins-base libid3tag"

inherit gettext

PACKAGE_ARCH = "${MACHINE_ARCH}"
COMPATIBLE_MACHINE = "(atlas)"

PACKAGECONFIG ??= " \
    orc\
    "

EXTRA_SPECIFIC_CONF = " \
    --enable-asfdemux \
    --disable-dvdlpcmdec \
    --disable-dvdsub \
    --enable-realmedia \
    --disable-xingmux \
    --disable-a52dec \
    "

EXTRA_OECONF += " \
    --disable-amrnb \
    --disable-amrwb \
    --disable-sidplay \
    --disable-twolame \
    ${GSTREAMER_1_0_ORC} \
    ${EXTRA_SPECIFIC_CONF} \
    "

