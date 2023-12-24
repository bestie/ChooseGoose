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
random_item_text="I'm Feeling Lucky"
excluded_systems_file="${HOME}/excluded_systems.txt"

log_file="$HOME/goose.log"
exec >> $log_file 2>&1
# exec >> /dev/null 2>&1
#
echo ""
echo ""

convert_to_basenames() {
    local filepaths="$1"
    for file in $filepaths; do
      echo "${file##*/}"
    done
}

find_roms() {
  local roms_dir="$1"
  local system_dir="$2"
  busybox find "${roms_dir}/${system_dir}" -type f -mindepth 1 -maxdepth 1 -name "*.*" ! -name "*.xml" ! -name "*.dat" -exec basename {} \;
}

select_random_item() {
  local items_list="$1"
  local count=$(echo "$items_list" | wc -l)
  local random_seed=$(date +%s)  # %s gives seconds since 1970-01-01 00:00:00 UTC
  local random_index=$(( random_seed % count + 1 ))
  echo "$items_list" | sed -n "${random_index}p"
}

filter_non_empty_dirs() {
  local dir_list="$1"
  local base_dir="$2"
  local non_empty_dirs=""

  for dir in $dir_list; do
    # echo "searching ${dir_prefix}/${dir}" >> $log_file
    busybox find "${base_dir}/${dir}" -type f -mindepth 1 -maxdepth 1 -name "*.*" ! -name "*.xml" ! -name "*.dat" -quit | grep -q .
    if [ $? -eq 0 ]; then
      # echo "found non empty ${dir_prefix}/${dir}" >> $log_file
      non_empty_dirs="${non_empty_dirs}\n${dir}"
    fi
  done

  echo -e "${non_empty_dirs#\\n}"
}

exclude_dirs() {
  local dir_list="$1"
  local exclusion_file="$2"
  echo "$dir_list" | grep -v -w -f "$exclusion_file"
}

# Choose a system ##############################################################
systems=$(./jq -r "keys[]" "${CORE_MAPPING}")
# echo "Systems: ${systems}"
systems_with_games=$(filter_non_empty_dirs "$systems" "$ROMS_DIR")
# echo "Systems with games: ${systems_with_games}"
selectable_systems=$(exclude_dirs "$systems_with_games" "$excluded_systems_file")
# echo "Selectable systems: ${selectable_systems}"
systems_menu=$(echo "$random_item_text" && echo "$selectable_systems")

system_selection=$(echo "$systems_menu" | ./choose_goose)

if [ "$system_selection" = "$random_item_text" ]; then
  echo "Choosing random system"
  system_selection=$(select_random_item "$selectable_systems")
fi
echo "Chosen system: ${system_selection}"

core_basename=$(./jq -r ".${system_selection}" $CORE_MAPPING)
core="${CORES_DIR}/${core_basename}"
echo "System core: ${core}"

# Choose a game ################################################################
# games=$(ls "${ROMS_DIR}/${system_selection}" | grep -v Imgs)
game_filepaths=$(find_roms "${ROMS_DIR}" "${system_selection}")
games=$(convert_to_basenames "$game_filepaths")
games_menu=$(echo "$random_item_text" && echo "$games")
echo "Got games menu"
game_selection=$(echo "$games_menu" | ./choose_goose)

if [ "$game_selection" = "$random_item_text" ]; then
    echo "Choosing random game"
    game_selection=$(select_random_item "$games")
fi
echo "Chosen game: ${game_selection}"

rom="${ROMS_DIR}/${system_selection}/${game_selection}"
echo "ROM file: ${rom}"

echo "******************** HONK! HONK! Here we fucking go ********************"

echo "Selected: $core -- $rom"
echo "~~~~~~~~~~~~~ Launching"
busybox sh ./game_launch.sh "${core}" "${rom}"
echo "~~~~~~~~~~~~~ Choosin' is done"
