SUMMARY = "GStreamer 1.0 multimedia framework"
DESCRIPTION = "GStreamer is a multimedia framework for encoding and decoding video and sound. \
It supports a wide range of formats including mp3, ogg, avi, mpeg and quicktime."
HOMEPAGE = "http://gstreamer.freedesktop.org/"
BUGTRACKER = "https://bugzilla.gnome.org/enter_bug.cgi?product=Gstreamer"
SECTION = "multimedia"
LICENSE = "LGPLv2+"
DEPENDS = "glib-2.0 libxml2 bison-native flex-native"

inherit autotools pkgconfig gettext

PACKAGE_ARCH = "${MACHINE_ARCH}"
COMPATIBLE_MACHINE = "(atlas)"

GSTREAMER_1_DEBUG ?= "--disable-debug"
EXTRA_OECONF = "--disable-docbook --disable-gtk-doc \
                --disable-dependency-tracking --disable-check \
                --disable-examples --disable-tests \
                --disable-valgrind ${GSTREAMER_1_DEBUG} \
                "

CACHED_CONFIGUREVARS += "ac_cv_header_valgrind_valgrind_h=no"

FILES_${PN} += " ${libdir}/gstreamer-1.0/*.so"
FILES_${PN}-dev += " ${libdir}/gstreamer-1.0/*.la ${libdir}/gstreamer-1.0/*.a"
FILES_${PN}-dbg += " ${libdir}/gstreamer-1.0/.debug/ ${libexecdir}/gstreamer-1.0/.debug/"

