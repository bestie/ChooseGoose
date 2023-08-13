#!/bin/sh
progdir=$(dirname "$0")/SimpleMenu
cd $progdir
HOME=$progdir
LD_PRELOAD=./j2k.so ./simple_menu
sync
