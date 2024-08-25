#!/bin/sh

################################################################################
#                                                                              #
#   ChooseGoose Simplified Video/Rom Launcher Demo                             #
#                                                                              #
#   Author: Stephen Best                                                       #
#   GitHub: https://github.com/bestie/ChooseGoose                              #
#                                                                              #
#   Description:                                                               #
#                                                                              #
#   Use a silly menu program to choose a file from a directory and launch      #
#   Retroarch.                                                                 #
#                                                                              #
#   GarlicOS doesn't provide an easy way to nest episodes of a TV series in    #
#   the VIDEOS directory. This script scans the a directory of mixed episodes  #
#   for show names and then again for the episodes of the chosen show.         #
#                                                                              #
#   The chosen video file is then played.                                      #
#                                                                              #
################################################################################

set -e

# Garlic 'apps' are shell scripts (like this one) in the APPS directory
# HOME is the app's directory with all its stuff in, it's easiest to cd into that directory.
HOME=$(busybox dirname "$0")/ChooseGoose
cd $HOME
RETROARCH_DIR=/mnt/mmc/CFW/retroarch

# Path to the Retroarch core for you content
CORE="/mnt/mmc/CFW/retroarch/.retroarch/cores/ffmpeg_libretro.so"
# Path to the content 
CONTENT_DIR="/mnt/SDCARD/Roms/VIDEOS"
title="Choose Goose Silly Video Browser"

# Get all the videos in the directory, filter out the Imgs directory
content_list=$(ls $CONTENT_DIR | grep -v Imgs)
# echo that into ChooseGoose and get the selection
selection=$(echo "$content_list" | ./choosegoose --title "$title" --text-color FF4500 --text-selected-color 00BAFF --text-selected-background-color 3BFF00 --background-color 333333 --hide-file-extensions true)

# Launch Retroarch
export HOME=$RETROARCH_DIR
${RETROARCH_DIR}/retroarch -L "${CORE}" "${CONTENT_DIR}/${selection}"
