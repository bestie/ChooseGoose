#!/bin/sh
set -e

progdir=$(dirname "$0")/ChooseGoose
HOME=$progdir

VIBES=/sys/class/power_supply/battery/moto
RETROARCH=/mnt/mmc/CFW/retroarch
game_launch_script="$RETROARCH/command.sh"

function cleanup {
  echo 0 > $VIBES
}
trap cleanup EXIT

echo 60 > $VIBES

# echo "******* game_launch.sh starting"
# printenv

while [ -f $game_launch_script ];
do
  echo "command file exists"
  sleep 1
done

echo "Retroarch command file is gone"
echo "Creating a new script to run"

cat <<EOF > $game_launch_script
#!/bin/sh
export SDL_NOMOUSE=1
export LANG=en_us
export HOME=/mnt/mmc/CFW/retroarch

${RETROARCH}/retroarch -L "${CORE}" "${ROM}"
exit $?
EOF

until ps_line=$(ps | grep -q "[./]main$")
do
  echo "ps: $ps_line"
  echo "main not running"
  sleep 0.5
done

echo $ps_line | busybox awk '{print $1}' | busybox xargs kill -TERM

echo 0 > $VIBES
