#!/bin/sh
set -eu
# Select the actual analog input. Default matches the board's initial mixer.
input=${1:-left1}
amixer -q -c 0 sset 'Capture' 35,35 cap
amixer -q -c 0 sset 'ADC PCM' 195,195
amixer -q -c 0 sset 'ALC Function' Off
for side in Left Right; do
    if [ "$side" = Left ]; then prefix=L; else prefix=R; fi
    amixer -q -c 0 sset "$side Input Mixer Boost" off
    for n in 1 2 3; do
        amixer -q -c 0 sset "$side Boost Mixer ${prefix}INPUT$n" off
        amixer -q -c 0 sset "$side Input Boost Mixer ${prefix}INPUT$n" 0
    done
done
case "$input" in
    left1)
        amixer -q -c 0 sset 'Left Input Mixer Boost' on
        amixer -q -c 0 sset 'Left Boost Mixer LINPUT1' on
        amixer -q -c 0 cset name='ADC Data Output Select' 1
        ;;
    right2)
        amixer -q -c 0 sset 'Right Input Mixer Boost' on
        amixer -q -c 0 sset 'Right Boost Mixer RINPUT2' on
        amixer -q -c 0 sset 'Right Input Boost Mixer RINPUT2' 3
        amixer -q -c 0 cset name='ADC Data Output Select' 2
        ;;
    *) echo 'Use left1 or right2' >&2; exit 2 ;;
esac
echo MIC_CONFIG_OK:$input
