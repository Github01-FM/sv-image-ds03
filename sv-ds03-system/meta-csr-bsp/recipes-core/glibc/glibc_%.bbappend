#use this to avoid the failure of murphy because lack of __ARM_PCS_VFP
#will root-cause it in the feature
do_fix_pcsvfp () {
#for nativesdk gcc, don't do "fix", ${D}/usr/include/gnu/stubs.h only
#exists for arm sysroot
	if [ -f ${D}/usr/include/gnu/stubs.h ]; then
		echo "guevara modify eglibc"
		sed -i '1i#define __ARM_PCS_VFP' ${D}/usr/include/gnu/stubs.h
	fi
}

addtask do_fix_pcsvfp after do_install before do_populate_sysroot do_package
