echo "Env setup :)"
export PATH="${PATH}:/opt/miyoo/bin:/opt/miyoo/arm-miyoo-linux-uclibcgnueabi/sysroot/usr/bin"
export CROSS_COMPILE=/opt/miyoo/bin/arm-miyoo-linux-uclibcgnueabi-
export PREFIX=/opt/miyoo/
export PLATFORM=rg35xx
export CC=${CROSS_COMPILE}gcc
export AR=${CROSS_COMPILE}ar
export AS=${CROSS_COMPILE}as
export LD=${CROSS_COMPILE}ld
export CXX=${CROSS_COMPILE}g++
export HOST=arm-miyoo-linux-uclibcgnueabi
