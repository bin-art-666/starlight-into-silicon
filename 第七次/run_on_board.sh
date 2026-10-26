#!/bin/sh
# 在板端本地目录运行；暂停原桌面，退出后恢复。
set -u
cd /home/root/lab7 || exit 1
desktop_pids=$(pidof QDesktop 2>/dev/null || true)
restore_desktop() {
    if [ -n "$desktop_pids" ]; then kill -CONT $desktop_pids 2>/dev/null || true; fi
}
trap restore_desktop EXIT
trap 'exit 130' INT TERM
if [ -n "$desktop_pids" ]; then kill -STOP $desktop_pids || exit 1; fi
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb0
export QT_QPA_FONTDIR=/usr/share/fonts/ttf
mkdir -p evidence
./qt_led_arm --evidence=/home/root/lab7/evidence "$@"
status=$?
exit "$status"
