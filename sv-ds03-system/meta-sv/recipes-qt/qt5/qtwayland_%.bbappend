
SRC_URI += " \
    file://sv001-disable-pointer.patch \
"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

PACKAGECONFIG ?= " \
    wayland-client \
    wayland-egl \
    "
