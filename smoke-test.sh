#/usr/bin/env bash

set -ue

executable="$(make echo-bin-path)"
echo "executable = $executable"

$executable --help | grep "ChooseGoose — a silly graphical menu system" \
  && echo "✅ Help output looks good"

choices=$(printf "Duck\nGoose\nMoose")
timeout=1000
expected_min_runtime=1400
log=/tmp/goose.log
rm -f $log
command="$executable --background-image=DEFAULT --user-inactivity-timeout-ms=$timeout --log-file=$log"

function on_error {
  echo "🐳🐳🐳🐳🐳🐳🐳"
  echo "🪿🪵🪵🪵🪵🪵🪵🪵🪵🪵🪵🪵🪵"
  cat $log
}

function delete_log {
  rm -f "$log"
}

trap on_error ERR
trap delete_log EXIT

TIMEFORMAT=%R
runtime=$( { time echo "$choices" | ($command 2>/dev/null || [ $? -eq 124 ]) 2>/dev/null; } 2>&1 )
exit_code=$?

echo "⏱️ runtime (s) = $runtime"
runtime_ms=$(echo "$runtime * 10000/10" | bc)
echo "⏱️ runtime (ms) = $runtime_ms"
test $runtime_ms -gt $expected_min_runtime && echo "🥱👍 That took a sufficiently long time."
test $exit_code -eq 0 && echo "🚪👍 Exit code was good. It was actually 124, but I caught that and wrapped it."

# Check the log
cat $log | head -n1 | grep "HONK HONK"        > /dev/null
cat $log | grep "SDL starting"                > /dev/null
cat $log | grep "Font loaded"                 > /dev/null
cat $log | grep "SDL waiting for event"       > /dev/null
cat $log | grep "blitting menu item"          > /dev/null
cat $log | grep "Inactivity timeout reached"  > /dev/null
echo "Log has stuff in it 👀👍"
