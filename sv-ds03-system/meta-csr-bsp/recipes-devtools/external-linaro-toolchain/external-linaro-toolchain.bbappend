FILES_libgfortran-staticdev = " \
  ${base_libdir}/libgfortran.a \
  ${base_libdir}/libgfortranbegin.a \
"
FILES_libmudflap-dev = "\
  ${base_libdir}/libmudflap*.so \
  ${base_libdir}/libmudflap*.la"
FILES_libmudflap-staticdev = "\
  ${base_libdir}/libmudflap*.a \
"

INSANE_SKIP_external-linaro-toolchain += "installed-vs-shipped"

FILES_${PN}_append = " ${base_libdir}/${ELT_TARGET_SYS}/*"

INSANE_SKIP_libgcov-dev = "staticdev"
FILES_libgcov-dev = "/lib/gcc/arm-linux-gnueabihf/4.9.2/libgcov.a"

RPROVIDES_${PN}_append = " rtld(GNU_HASH)"
#fix it to 2014.09 othwise glibc-dev will fail to install
PKGV_${PN} = "2014.09"

do_install_append () {

	cp -a ${EXTERNAL_TOOLCHAIN}/${ELT_TARGET_SYS}/libc/usr/bin/*  ${D}${bindir}
#guevara add these because original linaro-toolchain doesn't define __kernel_long_t
#and will fail to build packages like readline 
	sed -i '22i#ifndef __kernel_long_t' ${D}/usr/include/asm/posix_types.h
	sed -i '23itypedef long		__kernel_long_t;' ${D}/usr/include/asm/posix_types.h
	sed -i '24itypedef unsigned long	__kernel_ulong_t;' ${D}/usr/include/asm/posix_types.h
	sed -i '25i#endif' ${D}/usr/include/asm/posix_types.h

	sed -i '1i#define __ARM_PCS_VFP' ${D}/usr/include/gnu/stubs.h

#need to symlink it otherwise ld failed to find out this lib
	mkdir -p ${D}/lib/${ELT_TARGET_SYS}/
	ln -sf ../ld-2.19-2014.08.so ${D}/lib/${ELT_TARGET_SYS}/ld-linux-armhf.so.3
	ln -sf ../libanl-2.19-2014.08.so ${D}/lib/${ELT_TARGET_SYS}/libanl.so.1
#	ln -sf ../libasan.so.1.0.0 ${D}/lib/${ELT_TARGET_SYS}/libasan.so
	ln -sf ../libasan.so.1.0.0 ${D}/lib/${ELT_TARGET_SYS}/libasan.so.1
#	ln -sf ../libatomic.so.1.1.0 ${D}/lib/${ELT_TARGET_SYS}/libatomic.so
	ln -sf ../libatomic.so.1.1.0 ${D}/lib/${ELT_TARGET_SYS}/libatomic.so.1
	ln -sf ../libBrokenLocale-2.19-2014.08.so ${D}/lib/${ELT_TARGET_SYS}/libBrokenLocale.so.1
	ln -sf ../libcrypt-2.19-2014.08.so ${D}/lib/${ELT_TARGET_SYS}/libcrypt.so.1
	ln -sf ../libc-2.19-2014.08.so ${D}/lib/${ELT_TARGET_SYS}/libc.so.6
	ln -sf ../libdl-2.19-2014.08.so ${D}/lib/${ELT_TARGET_SYS}/libdl.so.2
#	ln -sf ../libgfortran.so.3.0.0 ${D}/lib/${ELT_TARGET_SYS}/libgfortran.so
	ln -sf ../libgfortran.so.3.0.0 ${D}/lib/${ELT_TARGET_SYS}/libgfortran.so.3
	ln -sf ../libgomp.so.1.0.0 ${D}/lib/${ELT_TARGET_SYS}/libgomp.so.1
	ln -sf ../libgomp.so.1.0.0 ${D}/lib/${ELT_TARGET_SYS}/libgomp.so.1
#	ln -sf ../libitm.so.1.0.0 ${D}/lib/${ELT_TARGET_SYS}/libitm.so
	ln -sf ../libitm.so.1.0.0 ${D}/lib/${ELT_TARGET_SYS}/libitm.so.1
	ln -sf ../libm-2.19-2014.08.so ${D}/lib/${ELT_TARGET_SYS}/libm.so.6
	ln -sf ../libnsl-2.19-2014.08.so ${D}/lib/${ELT_TARGET_SYS}/libnsl.so.1
	ln -sf ../libnss_compat-2.19-2014.08.so ${D}/lib/${ELT_TARGET_SYS}/libnss_compat.so.2
	ln -sf ../libnss_db-2.19-2014.08.so ${D}/lib/${ELT_TARGET_SYS}/libnss_db.so.2
	ln -sf ../libnss_dns-2.19-2014.08.so ${D}/lib/${ELT_TARGET_SYS}/libnss_dns.so.2
	ln -sf ../libnss_files-2.19-2014.08.so ${D}/lib/${ELT_TARGET_SYS}/libnss_files.so.2
	ln -sf ../libnss_hesiod-2.19-2014.08.so ${D}/lib/${ELT_TARGET_SYS}/libnss_hesiod.so.2
	ln -sf ../libnss_nisplus-2.19-2014.08.so ${D}/lib/${ELT_TARGET_SYS}/libnss_nisplus.so.2
	ln -sf ../libnss_nis-2.19-2014.08.so ${D}/lib/${ELT_TARGET_SYS}/libnss_nis.so.2
	ln -sf ../libpthread-2.19-2014.08.so ${D}/lib/${ELT_TARGET_SYS}/libpthread.so.0
	ln -sf ../libresolv-2.19-2014.08.so ${D}/lib/${ELT_TARGET_SYS}/libresolv.so.2
	ln -sf ../librt-2.19-2014.08.so ${D}/lib/${ELT_TARGET_SYS}/librt.so.1
#	ln -sf ../libssp.so.0.0.0 ${D}/lib/${ELT_TARGET_SYS}/libssp.so
	ln -sf ../libssp.so.0.0.0 ${D}/lib/${ELT_TARGET_SYS}/libssp.so.0
#	ln -sf ../libstdc++.so.6.0.20 ${D}/lib/${ELT_TARGET_SYS}/libstdc++.so
	ln -sf ../libstdc++.so.6.0.20 ${D}/lib/${ELT_TARGET_SYS}/libstdc++.so.6
	ln -sf ../libthread_db-1.0.so ${D}/lib/${ELT_TARGET_SYS}/libthread_db.so.1
#	ln -sf ../libubsan.so.0.0.0 ${D}/lib/${ELT_TARGET_SYS}/libubsan.so
	ln -sf ../libubsan.so.0.0.0 ${D}/lib/${ELT_TARGET_SYS}/libubsan.so.0
	ln -sf ../libutil-2.19-2014.08.so ${D}/lib/${ELT_TARGET_SYS}/libutil.so.1

#	rm ${D}/lib/libattr.*
#	rm ${D}/lib/libacl.*
#	rm ${D}/usr/lib/liblzma.*

#fix the populate_sdk issue for libgcov-dev
       mkdir -p ${D}/lib/gcc/arm-linux-gnueabihf/4.9.2/
       cp  ${EXTERNAL_TOOLCHAIN}/lib/gcc/arm-linux-gnueabihf/4.9.2/libgcov.a ${D}/lib/gcc/arm-linux-gnueabihf/4.9.2/

	cp -a ${EXTERNAL_TOOLCHAIN}/${ELT_TARGET_SYS}/include/* ${D}${includedir}

}

do_install_locale_append () {
	if [ -e ${D}${bindir}/locale ]; then
		mv -f ${D}${bindir}/locale ${dest}${bindir}
	fi
}
