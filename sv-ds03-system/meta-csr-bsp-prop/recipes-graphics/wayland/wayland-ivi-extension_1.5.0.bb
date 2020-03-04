SUMMARY = "Wayland IVI Extension"
DESCRIPTION = "GENIVI Layer Management API based on Wayland IVI Extension"
HOMEPAGE = "http://projects.genivi.org/wayland-ivi-extension"
BUGTRACKER = "http://bugs.genivi.org/enter_bug.cgi?product=Wayland%20IVI%20Extension"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=176cedb32f48dd58f07e0c1c717b3ea4"

DEPENDS = "weston"

# FIX ME
# This should be TAG = "${PV}" but yocto doesn't support lightweight tags for now
# https://bugzilla.yoctoproject.org/show_bug.cgi?id=6881
TAG = "8f0874b606b8e2a9385af947728905735bad3939"
SRC_URI = "git://git.projects.genivi.org/${PN}.git;tag=${TAG};protocol=http \
           file://0001-EGLWLMockNavigation-fix-ivalid-conversion-compile-er.patch \
          "
SRC_URI[md5sum] = "8de522ba3012d0878c7789979286e88c"
SRC_URI[sha256sum] = "1b5878fa54bb470b836ae91369a829912c1490a0dcd58024947b66153f61c983"
S = "${WORKDIR}/git"

inherit cmake autotools

FILES_${PN} += "${libdir}/weston/*"
FILES_${PN}-dbg += "${libdir}/weston/.debug/*"
