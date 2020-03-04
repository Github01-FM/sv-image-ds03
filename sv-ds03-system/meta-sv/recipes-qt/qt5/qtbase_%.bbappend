
#FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

DEPENDS += "wayland"

PACKAGECONFIG_GL = ""

PACKAGECONFIG_DEFAULT += "icu accessibility freetype fontconfig libinput gles2"

EXTRA_OECONF_append = " -qpa wayland"
