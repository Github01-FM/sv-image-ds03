#!/bin/bash

# author uidq1644 ganzixing
#
# date 2019-03-09
# last modify: 2019-06-18
#

# compile:
# 1:git clean -xdf 
# 2:source ~/.bashrc 
# 3:source env_setup.sh 
# 4:bitbake qtbase
# 5:bitbake qtdeclarative
# 6:bitbake qtmultimedia
# 7:bitbake qtwayland
# 8:bitbake qtquickcontrols
# 8:bitbake qtxmlpatterns


# Gets the current path
current_path=`pwd`

# compile or pack flag
c_or_p_flag=0

# chang assign version
# assign_version_flag=0

echo "Starting run ..."
echo " "

echo " "
source env_setup.sh

echo " "
pwd


echo "Starting compile ..."
#compile
bitbake qtbase && bitbake qtdeclarative && bitbake qtmultimedia && bitbake qtwayland && bitbake qtquickcontrols && bitbake qtxmlpatterns

echo " "
echo "Starting compile End..."
echo " "

rm -rf $current_path/../qt5.8

mkdir $current_path/../qt5.8


#copy so
cp $current_path/build/tmp/work/cortexa7hf-vfp-poky-linux-gnueabi/qtbase/5.8.0*/packages-split/qtbase/usr/lib/*.so.5.8.0 $current_path/../qt5.8

cp $current_path/build/tmp/work/cortexa7hf-vfp-poky-linux-gnueabi/qtdeclarative/5.8.0*/packages-split/qtdeclarative/usr/lib/*.so.5.8.0 $current_path/../qt5.8

cp $current_path/build/tmp/work/cortexa7hf-vfp-poky-linux-gnueabi/qtmultimedia/5.8.0*/packages-split/qtmultimedia/usr/lib/*.so.5.8.0 $current_path/../qt5.8

cp $current_path/build/tmp/work/cortexa7hf-vfp-poky-linux-gnueabi/qtwayland/5.8.0*/packages-split/qtwayland/usr/lib/*.so.5.8.0 $current_path/../qt5.8

cp $current_path/build/tmp/work/cortexa7hf-vfp-poky-linux-gnueabi/qtquickcontrols/5.8.0*/packages-split/qtquickcontrols/usr/lib/*.so.5.8.0 $current_path/../qt5.8

cp $current_path/build/tmp/work/cortexa7hf-vfp-poky-linux-gnueabi/qtxmlpatterns/5.8.0*/packages-split/qtxmlpatterns/usr/lib/*.so.5.8.0 $current_path/../qt5.8


echo "Done !!!"

exit
