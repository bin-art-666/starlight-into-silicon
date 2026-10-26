#!/bin/sh
# Run from /home/root/lab5. Existing recordings are preserved with timestamps.
set -eu
cd "$(dirname "$0")"
command -v aplay >/dev/null
command -v arecord >/dev/null
command -v amixer >/dev/null
command -v alsactl >/dev/null
if [ ! -f mixer-before.state ]; then
    alsactl -f mixer-before.state store
fi
sh playback_config.sh
sh mic_config.sh left1
if [ ! -f lab5_music.wav ]; then
    python3 make_music.py
fi
aplay -D hw:0,0 lab5_music.wav
echo 'Speak towards the on-board MIC during the next 10 seconds.'
record="mic_$(date +%Y%m%d_%H%M%S)_$$.wav"
arecord -D hw:0,0 -f cd -d 10 "$record"
aplay -D hw:0,0 "$record"
alsactl -f mixer-lab5.state store
echo "LAB5_SEQUENCE_OK recording=$record"
