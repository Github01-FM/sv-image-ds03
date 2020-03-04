PACKAGES += " locale"
FILES_locale = "${bindir}/locale /etc/profile.d/locale_env.sh"

#set the locales to genreate
GLIBC_GENERATE_LOCALES = "en_US.UTF-8 zh_CN.UTF-8"
DISTRO_FEATURES_LIBC += " libc-charsets libc-locales libc-locale-code"
ENABLE_BINARY_LOCALE_GENERATION = "1"

#The linaro prebuild locale and localedef's default locale folder is /usr/lib/arm-linux-gnueabihf/locale
localedir_forcevariable = "${libdir}/${ELT_TARGET_SYS}/locale"

#The linaro prebuild toolchain's name is glibc ,which is not recognized by the bb
#ln -sf it to eglibc
do_install_prepend () {
	echo "export LC_ALL=en_US.UTF-8" >> ${WORKDIR}/locale_env.sh
        install -D -m 0755 ${WORKDIR}/locale_env.sh ${D}/etc/profile.d/locale_env.sh
	ln -sf ${STAGING_INCDIR}/glibc-locale-internal-${MULTIMACH_TARGET_SYS} ${STAGING_INCDIR}/eglibc-locale-internal-${MULTIMACH_TARGET_SYS}
}

INSANE_SKIP_${PN} += " ldflags already-stripped installed-vs-shipped"


