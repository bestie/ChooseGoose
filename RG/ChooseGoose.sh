#!/bin/sh

progdir=$(dirname "$0")/ChooseGoose
cd $progdir
HOME=$progdir
# LD_LIBRARY_PATH=./lib
list=$(ls /mnt/SDCARD/Roms/MD)
LD_PRELOAD=./j2k.so echo $list | ./choose_goose
sync
