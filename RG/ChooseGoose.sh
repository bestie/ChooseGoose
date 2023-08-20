#!/bin/sh

progdir=$(dirname "$0")/ChooseGoose
cd $progdir
HOME=$progdir
LD_LIBRARY_PATH="${progdir}/lib"
list=$(ls /mnt/SDCARD/Roms/MD)
echo $list | ./choose_goose
sync
