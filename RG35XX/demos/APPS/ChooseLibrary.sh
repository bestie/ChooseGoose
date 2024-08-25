#!/bin/sh

###############################################################################
#                                                                              #
#   ChooseGoose Video Library Demo                                             #
#                                                                              #
#   Author: Stephen Best                                                       #
#   GitHub: https://github.com/bestie/ChooseGoose                              #
#                                                                              #
#   Description:                                                               #
#                                                                              #
#   Use a silly menu program to simulate a directory structure when all your   #
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

HOME=$(busybox dirname "$0")/ChooseGoose
cd $HOME
RETROARCH_DIR=/mnt/mmc/CFW/retroarch

# Path to the Retroarch core for you content
CORE="/mnt/mmc/CFW/retroarch/.retroarch/cores/ffmpeg_libretro.so"
# Path to the content 
CONTENT_DIR="/mnt/SDCARD/Roms/VIDEOS"
# ChooseGoose options to re-use
title="~~ Silly Video Library"
goose_opts="--title-font-size 28 --font-size 24 --text-color DD0000 --text-selected-color 00CC00 --hide-file-extensions true --top-padding 20"
background_image="./assets/ChooseGooseVideoLibrary640x480.png"

# List the TV Episodes, remove the season and episode numbers to create a list of shows
tv_shows=$(ls $CONTENT_DIR | grep -v Imgs | sed -E 's/[^\w][sS][0-9]{2}[eE][0-9]{2}.*//' | uniq)
# Select from the list of shows
tv_show=$(echo "$tv_shows" | ./choosegoose --title "$title: TV Shows ~~" --background-image "$background_image" --text-selected-background-color DDDDDD $goose_opts)
# Filter all episodes for just that show
episodes=$(ls $CONTENT_DIR | grep -v Imgs | grep "$tv_show")

# Easily hack in a GarlicOS-like Imgs directory
background_image="$CONTENT_DIR/Imgs/$tv_show.png"
# or
# background_image=DEFAULT

# Choose an episode
selection=$(echo "$episodes" | ./choosegoose --title "$title: Episodes ~~" --background-image "$background_image" $goose_opts)

# Launch Retroarch to play
export HOME=$RETROARCH_DIR
${RETROARCH_DIR}/retroarch -L "${CORE}" "${CONTENT_DIR}/${selection}"
