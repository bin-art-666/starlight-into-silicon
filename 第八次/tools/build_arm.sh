#!/bin/bash
set -euo pipefail
qt=/home/little/lab7-sdk
cv=/home/little/lab8-sdk/opencv-3.1.0
libs=/home/little/lab8-sdk/board-sysroot
lab=/mnt/d/Develop/Linux/嵌入式/第八次
export PATH="$qt/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf/bin:$PATH"
includes=""
for dir in "$cv"/modules/*/include; do includes="$includes $dir"; done
mkdir -p "$lab/build/arm" "$lab/bin"
cd "$lab/build/arm"
"$qt/configure-build/bin/qmake" "$lab/qt_camera/qt_camera.pro" \
    "OPENCV_INCLUDE_DIRS=$includes" "OPENCV_LIBDIR=$libs/usr/lib" \
    "QMAKE_LIBDIR+=$qt/board-sysroot/usr/lib $qt/board-sysroot/lib" \
    "QMAKE_LFLAGS+=-Wl,--sysroot=$qt/board-sysroot -Wl,-rpath-link,$libs/usr/lib -Wl,-rpath-link,$qt/board-sysroot/usr/lib -Wl,-rpath-link,$qt/board-sysroot/lib"
make -j4 > build.log 2>&1
cp qt_camera "$lab/bin/qt_camera_arm"
file qt_camera
arm-linux-gnueabihf-readelf --version-info qt_camera > "$lab/实测记录/ARM符号版本.txt"
