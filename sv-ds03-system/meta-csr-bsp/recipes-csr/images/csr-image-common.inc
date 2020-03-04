LICENSE = "Proprietary"
IMAGE_INSTALL_append = " qtbase-fonts qtbase-fonts-ttf-vera qtbase-fonts-ttf-dejavu qtbase-fonts-pfa qtbase-fonts-pfb qtbase-fonts-qpf"
IMAGE_INSTALL_append = " qtbase-plugins"
IMAGE_INSTALL_append = " qtdeclarative-tools qtdeclarative-qmlplugins qtmultimedia-plugins"
IMAGE_INSTALL_append = " qtwayland qtwayland-plugins"
IMAGE_INSTALL_append = " sgx-linux-km packagegroup-csr-qtapp packagegroup-csr-ivi"

IMAGE_INSTALL_append = " gstreamer1.0 gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav"
IMAGE_INSTALL_append = " gst-plugins-base-all gst-plugins-good-all gst-plugins-bad-all gst-plugins-ugly-all gst-plugins-audecoder-all audiocodec-all gst-plugins-filebufsrc-all mp3aacconvert-all"
IMAGE_INSTALL_append = " vdec-km vdec-um vdec-um-all kernel-module-vxdkernel gst-omx"
IMAGE_INSTALL_append = " mediainfo"

IMAGE_INSTALL_append = " csr-udev-conf udev-extraconf"
IMAGE_INSTALL_append = " misc-tools"

IMAGE_INSTALL_append = " alsa-utils alsa-plugins"
IMAGE_INSTALL_append = " pulseaudio lua"
IMAGE_INSTALL_append = " devkit"

IMAGE_INSTALL_append = " wayland weston"
IMAGE_INSTALL_append = " wayland-ivi-extension"
IMAGE_INSTALL_append = " licensemgr"
IMAGE_INSTALL_append = " snps-eth"
IMAGE_INSTALL_append = " backup"
IMAGE_INSTALL_append = " tslib tslib-calibrate tslib-tests"

IMAGE_INSTALL_append = " locale localedef glibc-binaries"
