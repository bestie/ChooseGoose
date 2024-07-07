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
#   video files are just all dumped in one place.                              #
#                                                                              #
#   Initial menu is provided by the contents of a file named `.choosegoose.txt`#
#   which lists the TV shows that would otherwise be in directories.           #
#   This is manual, put what you like in here.                                 #
#                                                                              #
#   That selection is then used to filter the directory contents to allow the  #
#   user to choose from a filtered list of files.                              #
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
title="Silly Video Browser"

# Get all the videos in the directory, filter out the Imgs directory
content_list=$(ls $CONTENT_DIR | grep -v Imgs)
# echo that into ChooseGoose and get the selection
selection=$(echo "$content_list" | ./choosegoose --start-at-nth 1 --title "$title" --font-size 22)

# Launch Retroarch
export HOME=$RETROARCH_DIR
${RETROARCH_DIR}/retroarch -L "${CORE}" "${CONTENT_DIR}/${selection}"
