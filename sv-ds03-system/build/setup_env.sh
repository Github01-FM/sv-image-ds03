#!/bin/bash
#Note:Please put it under the poky/build/ directory
#Usage1: rm the build/conf and run TEMPLATECONF=meta-csr/conf source oe-init-build-env
#cd poky/build/
#./setup_env.sh
#Usage2: rm the conf sstate-cache cache tmp under build dir 
#cd poky/build/
#./setup_env.sh cleanall
##############################################################################

if [ ! -d ../build ];then 
	echo "#######################################################"
	echo "   This script should run under poky/build/"
	echo "   Please put  setup_env.sh under poky/build/"	
	echo "#######################################################"
	exit 1;
fi

echo "Usage1: rm the build/conf and run TEMPLATECONF=meta-csr/conf source oe-init-build-env"
echo "cd poky/build/"
echo "./setup_env.sh"
echo "Usage2: rm the conf sstate-cache cache tmp under build dir "
echo "cd poky/build/"
echo "./setup_env.sh cleanall"

if [ x$1 == "xcleanall" ];then
	echo "Now rm sstate-cache"
	rm -rf ./sstate-cache

	echo "Now rm cache"
	rm -rf ./cache
	
	echo "Now rm tmp"
	rm -rf ./tmp

	echo "Now rm -rf conf"
	rm -rf ./conf

	echo "Env has been cleaned,please run ./setup_env.sh to setup env" 
	exit 0;
fi

echo "Now rm -rf conf"
rm -rf ./conf

cd ..  && TEMPLATECONF=meta-csr/conf source ./oe-init-build-env build
echo "TEMPLATECONF=meta-csr/conf source oe-init-build-env have been executed"
echo "You can use bitbake to build your target now"





