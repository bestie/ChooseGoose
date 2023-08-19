#!/bin/sh
progdir=$(dirname "$0")/ChooseGoose
cd $progdir
HOME=$progdir
LD_PRELOAD=./j2k.so ./choose_goose
sync
