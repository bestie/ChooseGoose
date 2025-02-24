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
#   Use a silly menu program to choose a Mega Drive ROM from directory and     #
#   launch Retroarch.                                                          #
#                                                                              #
################################################################################

set -e

HOME=$(busybox dirname "$0")/ChooseGoose
cd $HOME
RETROARCH_DIR=/mnt/mmc/CFW/retroarch

# Path to the Retroarch core for you content
CORE="/mnt/mmc/CFW/retroarch/.retroarch/cores/picodrive_libretro.so"

# Path to the content
CONTENT_DIR="/mnt/SDCARD/Roms/MD"

# ChooseGoose options to re-use
title="  ~~ Sega Mega Drive / Genesis ~~"
style_opts="--title-font-size 28 --font-size 20 --text-color DD0000 --text-selected-color 00CC00 --text-selected-background-color CC00CC --hide-file-extensions true --top-padding 20 --background-image DEFAULT --menu-item-padding 1 --menu-item-margin 1"

roms=$(ls $CONTENT_DIR | grep -v Imgs)
selection=$(echo "$roms" | ./choosegoose --title "$title" --cover-images-dir=/mnt/SDCARD/Roms/MD/Imgs --log-file $HOME/log $style_opts)

# Launch Retroarch to play
export HOME=$RETROARCH_DIR
${RETROARCH_DIR}/retroarch -L "${CORE}" "${CONTENT_DIR}/${selection}"
