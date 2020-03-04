#Yocto use SERIAL_CONSOLES = "115200;ttySiRF1" to generate the label for inittab
#but SiRF1 is too long(>4) for inittab, so remove the SiRF.
do_install_append () {
	sed -i 's/^SiRF//' ${D}/etc/inittab
}
