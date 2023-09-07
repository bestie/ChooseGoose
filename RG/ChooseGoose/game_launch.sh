#!/bin/sh

# game_launcher.sh - a general purpose game and app launcher for GarlicOS.
# Author: Stephen Best github.com/bestie
#
# This shell script is a hack around how the GarlicOS start script
# (/mnt/mmc/CFW/start) launches the main menu, games, and apps.
#
# Simplifed:
#   - start runs in a loop
#   - launches the main menu (executable also called main)
#   - on selecting a game or app a temporary shell script is written to the Retroarch directory
#   - main quits
#   - start looks for a script to run and runs it
#   - the temporary script launches the game
#   - when the game exits the start loop continues
#   - the temporary script is deleted
#   - the main menu is restarted
#
# game_launch.sh
#   - respawns in background
#   - original process exits
#   - parent app / script can exit allow start script loop to continue
#   - game_launch.sh waits for the temporary script to be deleted
#   - start script deletes the temporary script
#   - game_launch.sh creates another one to launch its own game
#   - start script may detect the new temp script and launch immeditely
#   - game_launch.sh waits to see if it won the race with start loop
#   - if game_launch.sh was too late start will launch main again
#   - game_launch.sh then kills main
#   - start then launches the game

VIBES=/sys/class/power_supply/battery/moto
RETROARCH=/mnt/mmc/CFW/retroarch
log_file="$HOME/goose.log"
garlic_game_launch_script="$RETROARCH/command.sh"
NO_HUP=${NO_HUP-"NO NO_HUP"}

# exec >> $log_file 2>&1
exec >> /dev/null 2>&1

core="$1"
rom="$2"

function cleanup {
  echo "game_launch.sh exiting"
  echo 0 > $VIBES
}
trap cleanup EXIT


function respawn_in_background {
  if [ "$NO_HUP" != "NO HUPS GIVEN" ]
  then
    echo "Respawning in background"
    export NO_HUP="NO HUPS GIVEN"
    busybox nohup "$0" "$core" "$rom" > /dev/null 2>&1 &
    exit 0
  else
    echo "Already running in background"
  fi
}

function give_a_little_honk {
  # HONK HONK
  echo 60 > $VIBES
  sleep 0.2
  echo 0 > $VIBES
  sleep 0.5
  echo 60 > $VIBES
  sleep 0.2
  echo 0 > $VIBES
}

function wait_for_temp_launch_script_to_be_deleted {
  attempts=0
  max_attempts=5

  while [ -f "$garlic_game_launch_script" ] && [ $attempts -lt $max_attempts ]; do
    echo "Retroarch launch script still exists"
    sleep 0.5
    attempts=$((attempts + 1))
  done

  if [ $attempts -eq $max_attempts ]; then
    echo "Error: Retroarch launch script was not removed, can't launch another game."
    exit 1
  fi

  echo "Retroarch script has been deleted"
}

function write_new_temp_retroarch_script {
cat <<EOF > $garlic_game_launch_script
#!/bin/sh
export SDL_NOMOUSE=1
export LANG=en_us
export HOME=/mnt/mmc/CFW/retroarch

${RETROARCH}/retroarch -L "${core}" "${rom}"
exit $?
EOF

cat $garlic_game_launch_script

}

function kill_main_if_running {
  echo "kill main if running"
  ps | grep "main" | grep -v "grep"

  attempts=0
  max_attempts=5

  while [[ $attempts -lt $max_attempts ]]; do
    ps_line=$(ps | grep "main" | grep -v "grep")
    echo "checking for main process ${ps_line} $?"
    ps | grep "main" | grep -v "grep"
    
    if [[ "$ps_line" == "" ]]; then
      echo "ps: $ps_line"
      echo "main not running $attempts"
      sleep 0.5
      attempts=$((attempts+1))
    else
      echo "Killing main process"
      echo $ps_line | busybox awk '{print $1}'
      echo $ps_line | busybox awk '{print $1}' | busybox xargs kill -TERM
      break
    fi
  done
}

echo "game_launch.sh starting"
echo "NO_HUP = $NO_HUP"
# printenv

respawn_in_background
give_a_little_honk &
wait_for_temp_launch_script_to_be_deleted
write_new_temp_retroarch_script
kill_main_if_running

echo "game_launch.sh done"
