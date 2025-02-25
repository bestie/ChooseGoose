#!/bin/sh

###############################################################################
#                                                                              #
#   ChooseGoose ROM launcher demo with cover images                            #
#                                                                              #
#   Author: Stephen Best                                                       #
#   GitHub: https://github.com/bestie/ChooseGoose                              #
#                                                                              #
#   Description:                                                               #
#                                                                              #
#   Use a silly menu program to choose a system then a game to play which is   #
#   then launched in Retroarch.                                                #
#                                                                              #
#   This demo displays a list of systems with 'friendly' names that can be     #
#   found in a JSON file.                                                      #
#                                                                              #
#   The game list dispays cover images from the Imgs directory like GarlicOS.  #
#                                                                              #
################################################################################

HOME=$(busybox dirname "$0")/ChooseGoose
cd $HOME
log=$HOME/log
RETROARCH_DIR=/mnt/mmc/CFW/retroarch
ROMS="/mnt/SDCARD/Roms"

core_mappings_file=/mnt/mmc/CFW/config/coremapping.json
friendly_mappings_file="$HOME/friendlymapping.json"

if [[ ! -d "$ROMS" ]]; then
    echo "Error: ROMS directory not found!" >> $log
    exit 1
fi
if [[ ! -f "$core_mappings_file" ]]; then
    echo "Error: Friendly name mapping file ($FRIENDLY_MAPPING) not found!" >> $log
    exit 1
fi
if [[ ! -f "$friendly_mappings_file" ]]; then
    echo "Error: Friendly name mapping file ($FRIENDLY_MAPPING) not found!" >> $log
    exit 1
fi

if [[ ! -f "$HOME/jq" ]]; then
    echo "Error: jq not found!" >> $log
    exit 1
fi

get_friendly_name() {
    system_id="$1"
    $HOME/jq --raw-output ".\"$system_id\"" $friendly_mappings_file
}

get_system_id() {
    friendly_NAME="$1"
    $HOME/jq --raw-output ". | to_entries[] | select(.value == \"$friendly_NAME\") | .key" $friendly_mappings_file
}

get_core() {
    SYSTEM_ID="$1"
    $HOME/jq --raw-output ".\"$SYSTEM_ID\"" $core_mappings_file
}

lauch_retroarch() {
    CORE="$1"
    CONTENT="$2"
    export HOME=$RETROARCH_DIR
    ${RETROARCH_DIR}/retroarch -L "${CORE}" "${CONTENT}"
}

system_ids=$($HOME/jq 'keys[]' --raw-output $core_mappings_file)
system_list=""
for system_id in $system_ids; do
    echo "checking $ROMS/$system_id"
    if [ -d "$ROMS/$system_id" ]; then
        # Counting the files is too slow! 😭
        # Check if directory contains any files (excluding empty directories)
        # file_count=$(find "$ROMS/$system_id" -type f -quit | grep . | wc -l)
        # if [ "$file_count" -gt 0 ]; then
            echo "$ROMS/$system_id found $file_count files"
            friendly_NAME=$(get_friendly_name "$system_id")
            system_list="$system_list\n$friendly_NAME"
            # system_list="$system_list\n$friendly_NAME ($file_count)"
        # fi
    else
        echo "Directory not found: $ROMS/$system_id"
    fi
done

# Check if any systems were found
if [ -z "$system_list" ]; then
    echo "No available systems with content found."
    exit 0
fi

# ChooseGoose options to re-use
style_opts="--title-font-size 28 --font-size 20 --text-color DD0000 --text-selected-color 00CC00 --text-selected-background-color CC00CC --hide-file-extensions true --top-padding 20 --background-image DEFAULT --menu-item-padding 1 --menu-item-margin 1"

# Loop to allow the user to go back to the system selection
while true; do
  title="  ~~ Choose a system ~~  "
  selection=$(echo -e "$system_list" | ./choosegoose --title "$title" --cover-images-dir=/mnt/SDCARD/Roms/MD/Imgs --log-file $HOME/log $style_opts)
  selected_system_id=$(get_system_id "$selection")
  core=$(get_core "$selected_system_id")
  content_dir="$ROMS/$selected_system_id"
  content_list=$(ls $content_dir | grep -v Imgs | grep -v .xml)
  title="  ~~ $system_friendly_name ~~  "
  rom_selection=$(echo "$content_list" | ./choosegoose --title "$title" --cover-images-dir=$content_dir/Imgs --log-file $HOME/log $style_opts)

  if [ $? -eq 0 ]; then
    break
  fi
done

lauch_retroarch "$core" "$content_dir/$rom_selection"
