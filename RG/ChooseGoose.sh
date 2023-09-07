#!/bin/sh
set -e

VIBES=/sys/class/power_supply/battery/moto
echo 0 > $VIBES

progdir=$(dirname "$0")/ChooseGoose
cd $progdir
HOME=$progdir
LD_LIBRARY_PATH="${progdir}/lib"
RETROARCH=/mnt/mmc/CFW/retroarch
ROMS_DIR="/mnt/SDCARD/Roms"
CORES_DIR="${RETROARCH}/.retroarch/cores"
CORE_MAPPING="/mnt/mmc/CFW/config/core_mapping.json"

log_file="$HOME/goose.log"
exec >> $log_file 2>&1
# exec >> /dev/null 2>&1

# Choose a system
system_selection=$(./jq -r "keys[]" "${CORE_MAPPING}" | ./choose_goose)
echo $system_selection
core_basename=$(./jq -r ".${system_selection}" $CORE_MAPPING)
core="${CORES_DIR}/${core_basename}"
echo $core

# Choose a game
game_selection=$(ls "${ROMS_DIR}/${system_selection}" | ./choose_goose)
rom="${ROMS_DIR}/${system_selection}/${game_selection}"

echo "************* HONK! "
echo "$(date)"
echo "$(pwd)"

echo "selected: $core -- $rom"
echo "~~~~~~~~~~~~~ Launching"
busybox sh ./game_launch.sh "${core}" "${rom}"
echo "~~~~~~~~~~~~~ Choosin' is done"
