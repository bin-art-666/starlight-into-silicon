#!/bin/sh
set -eu
amixer -q -c 0 sset 'Playback' 230,230
amixer -q -c 0 sset 'Headphone Playback ZC' on
amixer -q -c 0 sset 'Headphone' 105,105
amixer -q -c 0 sset 'Speaker Playback ZC' on
amixer -q -c 0 sset 'Speaker' 110,110
amixer -q -c 0 sset 'Speaker AC' 3
amixer -q -c 0 sset 'Speaker DC' 3
amixer -q -c 0 sset 'Right Output Mixer PCM' on
amixer -q -c 0 sset 'Left Output Mixer PCM' on
amixer -q -c 0 sset 'Right Output Mixer Boost Bypass' off
amixer -q -c 0 sset 'Left Output Mixer Boost Bypass' off
echo PLAYBACK_CONFIG_OK
