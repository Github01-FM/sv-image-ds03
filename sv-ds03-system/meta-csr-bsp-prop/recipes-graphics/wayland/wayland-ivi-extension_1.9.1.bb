SUMMARY = "Wayland IVI Extension"
DESCRIPTION = "GENIVI Layer Management API based on Wayland IVI Extension"
HOMEPAGE = "http://projects.genivi.org/wayland-ivi-extension"
BUGTRACKER = "http://bugs.genivi.org/enter_bug.cgi?product=Wayland%20IVI%20Extension"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=176cedb32f48dd58f07e0c1c717b3ea4"

PR = "r1"

SRC_URI = "git://github.com/GENIVI/${BPN}.git;protocol=http;rev=44598504503eea5ac7f94c88477a5a78bda01f30 \
    file://0001-EGLWLMockNavigation-fix-ivalid-conversion-compile-er.patch \
    "

DEPENDS = "weston"
S = "${WORKDIR}/git"

inherit cmake autotools

EXTRA_OECMAKE := "-DWITH_ILM_INPUT=1"

FILES_${PN} += "${libdir}/weston/*"
FILES_${PN}-dbg += "${libdir}/weston/.debug/*"
