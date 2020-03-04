SUMMARY = "Weston, a Wayland compositor"
DESCRIPTION = "Weston is the reference implementation of a Wayland compositor, add some specific defines"

FILESEXTRAPATHS_prepend := "${THISDIR}/weston:"
SRC_URI += "file://0001-weston-test-disable-egl-in-weston-test.patch"
SRC_URI += "file://0002_wayland_support_zero_copy_for_weston.patch"
SRC_URI += "file://0003_wayland_support_NV12_format_for_weston.patch"
SRC_URI += "file://0004-gl-renderer-update-dma-image-texture-every-damage.patch"
SRC_URI += "file://0005_ion_free_the_fd_after_import_it.patch"
SRC_URI += "file://0006_wayland_support_YUYV_format_for_weston.patch"
SRC_URI += "file://0007_Add_license_declaration_for_csr_modification.patch"
SRC_URI += "file://0008-ivi-shell-add-screenshot-function.patch"
SRC_URI += "file://0009-weston-Don-t-return-if-shm-buffer-exist-right-now.patch"
SRC_URI += "file://0010-disable-drm-support.patch"
SRC_URI += "file://0011-mv-egl_image-to-egl_image_dmabuf-for-dma-buffer-imag.patch"
SRC_URI += "file://0010-add-overlay-support-and-hw-vsync-for-weston.patch"
SRC_URI += "file://0011-rename-fb_fd-to-ion_fd.patch"
SRC_URI += "file://0012-add-interface-video-and-two-fields-format-support.patch"
SRC_URI += "file://0014-fix-bug-that-setting-src-dst-rect-has-wrong-effect.patch"
SRC_URI += "file://0015-correct-the-format-map-between-wayland-format-and-vd.patch"
SRC_URI += "file://0016-limit-to-support-one-yuv-overlay-pipe.patch"
SRC_URI += "file://0017-struct-fbdev-replace-fbcompositoir-in-version-1.9.patch"
SRC_URI += "file://0018-Add-g2d-renderer.patch"
SRC_URI += "file://0019-fix-crash-when-touchscreen-generates.patch"

SRC_URI += "file://weston.ini"
SRC_URI += "file://weston_env.sh"
SRC_URI += "file://weston.ini.ivi"

PACKAGECONFIG = "fbdev egl libinput"

FILES_${PN} += "/home/root/.config/weston.ini /etc/profile.d/weston_env.sh"
FILES_${PN} += "/home/root/.config/weston.ini.ivi"

#install weston.ini to home/root/
do_install_append () {
        install -D -m 0644 ${WORKDIR}/weston.ini ${D}/home/root/.config/weston.ini
        install -D -m 0755 ${WORKDIR}/weston_env.sh ${D}/etc/profile.d/weston_env.sh
        install -D -m 0644 ${WORKDIR}/weston.ini.ivi ${D}/home/root/.config/weston.ini.ivi
        rm ${D}/usr/lib/weston/weston-keyboard

}


