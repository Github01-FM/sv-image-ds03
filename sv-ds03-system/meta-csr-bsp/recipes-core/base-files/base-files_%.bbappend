do_install_prepend () {
	if [ x`grep "LD_LIBRARY_PATH" ${WORKDIR}/profile` = x ]; then
		echo "export LD_LIBRARY_PATH=/usr/lib" >> ${WORKDIR}/profile
	fi
	echo "tmpfs	/media	tmpfs	defaults	0	0" >> ${WORKDIR}/fstab
}
