SUMMARY = "Wayland, a protocol between a compositor and clients"
DESCRIPTION = "Wayland is a protocol for a compositor to talk to its clients \
as well as a C library implementation of that protocol. The compositor can be \
a standalone display server running on Linux kernel modesetting and evdev \
input devices, an X application, or a wayland client itself. The clients can \
be traditional applications, X servers (rootless or fullscreen) or other \
display servers."


FILESEXTRAPATHS_prepend := "${THISDIR}/wayland:"

SRC_URI += "file://0002_wayland_support_zero_copy_for_wayland.patch"
SRC_URI += "file://0003_wayland_support_NV12_format_for_wayland.patch"
SRC_URI += "file://0004_wayland_support_YUYV_format_for_wayland.patch"
SRC_URI += "file://0005_Add_license_declaration_for_csr_modification.patch"
SRC_URI += "file://0006-support-overlay-add-sync-event.patch"
SRC_URI += "file://0007-add-two-special-format-support.patch"

FILES_${PN} += "/usr/include/license-csr.h"
do_install_append () {
        install -D -m 0644 ${S}/src/license-csr.h ${D}/usr/include/license-csr.h
}


