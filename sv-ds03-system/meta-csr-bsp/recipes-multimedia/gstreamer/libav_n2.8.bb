require libav.inc

LIC_FILES_CHKSUM = "file://COPYING.LGPLv2.1;md5=bd7a443320af8c812e4c18d1b79df004"

SRC_URI = "git://source.codeaurora.org/quic/atlas7/multimedia/libav;protocol=https;branch=ffmpeg.org/release/2.8"
S = "${WORKDIR}/git"

SRCREV = "40934e0e9b632fa6c6ec22ac03b530625a027c79"
#SRCREV = "${AUTOREV}"

DEFAULT_PREFERENCE = "-1"
INSANE_SKIP_${PN} += " ldflags already-stripped installed-vs-shipped"
