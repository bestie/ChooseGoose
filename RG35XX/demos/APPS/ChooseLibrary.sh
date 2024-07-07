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
goose_opts="--text-color 00CC00 --font-size 28 --hide-file-extensions 0 --top-padding 20 --bottom-padding 80"

# Pick a line from the .choosegoose.txt which will be the TV Show we want
tv_show=$(cat $CONTENT_DIR/.choosegoose.txt | ./choosegoose $goose_opts --title "$title: TV Shows ~~" --background-image ./assets/ChooseGooseVideoLibrary640x480.png)

# Get the directory listing and filter for the name of the TV Show, remove the Imgs directory
episodes=$(ls $CONTENT_DIR | grep -v Imgs | grep "$tv_show")

# Easily hack in a GarlicOS-like Imgs directory
background_image="$CONTENT_DIR/Imgs/$tv_show.png"
# or
# background_image=none

# Pipe that into ChooseGoose and get the selection
selection=$(echo "$episodes" | ./choosegoose --title "$title: Episodes ~~" --background-image "$background_image" $goose_opts)

# Launch Retroarch
export HOME=$RETROARCH_DIR
${RETROARCH_DIR}/retroarch -L "${CORE}" "${CONTENT_DIR}/${selection}"
