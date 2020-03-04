
SRC_URI += " \
    file://sv001-enable-kIgnoreGpuBlacklist-switch.patch \
    "

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

# Enable examples
QTWEBENGINE_BUILD_PARTS ?= "examples"
EXTRA_QMAKEVARS_PRE += "QT_BUILD_PARTS+=${QTWEBENGINE_BUILD_PARTS}"
