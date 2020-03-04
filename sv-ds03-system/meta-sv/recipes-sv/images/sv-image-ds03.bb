LICENSE = "Proprietary"
#IMAGE_INSTALL_append = " qtbase-fonts qtbase-fonts-ttf-vera qtbase-fonts-ttf-dejavu qtbase-fonts-pfa qtbase-fonts-pfb qtbase-fonts-qpf"
IMAGE_INSTALL_append = " qtbase qtbase-plugins"
IMAGE_INSTALL_append = " qtdeclarative-tools qtdeclarative-qmlplugins qtmultimedia-plugins"
IMAGE_INSTALL_append = " qtwayland qtwayland-plugins"
IMAGE_INSTALL_append = " qtdeclarative qtdeclarative-plugins qtxmlpatterns qtquickcontrols-qmlplugins"


replace_init_with_fastrvcinit () {
      ln -nfs /usr/sbin/init-process ${IMAGE_ROOTFS}/sbin/init
}

ROOTFS_POSTPROCESS_COMMAND += "replace_init_with_fastrvcinit ; "

replace_init_with_libilmClient () {
      ln -nfs /usr/lib/libilmClient.so.1.9.1 ${IMAGE_ROOTFS}/usr/lib/libilmClient.so
}
ROOTFS_POSTPROCESS_COMMAND += "replace_init_with_libilmClient ; "

inherit core-image

