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
#   video files are just all dumped in flat directory.                         #
#                                                                              #
#   This demo compiles a list of TV shows based on the filenames of the        #
#   individual episodes. After selecting a TV show, it displays a filtered     #
#   list of episides just for that show.                                       #
#   
#                                                                              #
################################################################################

set -e

HOME=$(busybox dirname "$0")/ChooseGoose
cd $HOME
RETROARCH_DIR=/mnt/mmc/CFW/retroarch

# Path to the Retroarch core for you content
CORE="/mnt/mmc/CFW/retroarch/.retroarch/cores/ffmpeg_libretro.so"
# Path to the content 
CONTENT_DIR="/mnt/mmc/Roms/TV"
# ChooseGoose options to re-use
title="Silly Video Library"
text_color="446C7A"
text_selected_color="F8E4A5"
text_selected_background_color="323232"
goose_opts="--font $HOME/assets/iMWritingDuoNerdFontPropo-Bold.ttf --title-font-size 33 --font-size 30 --text-color "$text_color" --text-selected-color $text_selected_color --text-selected-background-color $text_selected_background_color --hide-file-extensions true --left-padding 10 --top-padding 10 --menu-item-padding 3 --menu-item-margin 0"
background_image="./assets/VCR.png"

# List the TV Episodes, remove the season and episode numbers to create a list of shows
tv_shows=$(ls $CONTENT_DIR | grep -v Imgs | sed -E 's/[^\w][sS][0-9]{2}[eE][0-9]{2}.*//' | uniq)
# Select from the list of shows
tv_show=$(echo "$tv_shows" | ./choosegoose --title "$title: TV Shows" --background-image "$background_image" $goose_opts)
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
