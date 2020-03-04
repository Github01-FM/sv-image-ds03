do_generate_content_prepend() {
    d.setVar('TARGET_CFLAGS', " -isystem$SDKTARGETSYSROOT/usr/include " + d.getVar('SELECTED_OPTIMIZATION', True))
    d.setVar('TARGET_CXXFLAGS'," -isystem$SDKTARGETSYSROOT/usr/include/c++/4.9.2/arm-linux-gnueabihf -isystem$SDKTARGETSYSROOT/usr/include/c++/4.9.2 -isystem$SDKTARGETSYSROOT/usr/include " + d.getVar('SELECTED_OPTIMIZATION', True))
    d.setVar('TARGET_LDFLAGS'," -L$SDKTARGETSYSROOT/lib -Wl,-rpath-link,$SDKTARGETSYSROOT/lib -Wl,-O1 " + d.getVar('TARGET_LINK_HASH_STYLE', True))
    d.setVar('TARGET_CPPFLAGS'," -isystem$SDKTARGETSYSROOT/usr/include/c++/4.9.2/arm-linux-gnueabihf -isystem$SDKTARGETSYSROOT/usr/include/c++/4.9.2 -isystem$SDKTARGETSYSROOT/usr/include ")
}
