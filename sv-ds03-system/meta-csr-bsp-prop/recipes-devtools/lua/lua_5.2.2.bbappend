FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI_append = "file://lua-5.2.2-shared_library-1.patch"

#murphy pkgconfig depends on this extra options,otherwise compile will fail
do_configure_append () {
        sed -i -e s:-llua$:"-llua -lm -ldl":g  ${S}/../lua.pc
        sed -i -e s:-I\$\{includedir\}$:"-I\$\{includedir\}\/":g  ${S}/../lua.pc 
}
