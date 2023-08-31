#!/bin/sh
set -e

VIBES=/sys/class/power_supply/battery/moto
# echo 0 > $VIBES

progdir=$(dirname "$0")/ChooseGoose
cd $progdir
HOME=$progdir
log_file="$HOME/goose.log"
LD_LIBRARY_PATH="${progdir}/lib"

RETROARCH=/mnt/mmc/CFW/retroarch
ROM_DIR="/mnt/SDCARD/Roms/MD"

selection=$(ls "${ROM_DIR}" | ./choose_goose)

echo "************* HONK! " >> $log_file
echo "$(date)" >> $log_file
echo "$(pwd)" >> $log_file
echo $selection >> $log_file

export CORE="${RETROARCH}/.retroarch/cores/picodrive_libretro.so"
export ROM="${ROM_DIR}/${selection}"

echo "$CORE / $ROM" >> $log_file
busybox nohup ./game_launch.sh > $log_file 2>&1 &
echo "~~~~~~~~~~~~~ Choosin' is done" >> $log_file
