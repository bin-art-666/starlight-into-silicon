#!/bin/bash
set -euo pipefail
sdk=/home/little/lab7-sdk
src=$sdk/qtbase-everywhere-src-5.12.9
tc=$sdk/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf
mkdir -p "$src/mkspecs/linux-lab7-g++" "$sdk/configure-build"
sed 's/arm-linux-gnueabi-/arm-linux-gnueabihf-/g' "$src/mkspecs/linux-arm-gnueabi-g++/qmake.conf" > "$src/mkspecs/linux-lab7-g++/qmake.conf"
cp "$src/mkspecs/linux-arm-gnueabi-g++/qplatformdefs.h" "$src/mkspecs/linux-lab7-g++/qplatformdefs.h"
# Qt 5.12 的主机工具在 GCC 13 下需显式包含 limits。
for h in "$src/src/corelib/global/qfloat16.h" "$src/src/corelib/global/qendian.h"; do
    if ! grep -q '^#include <limits>' "$h"; then sed -i '/#include <QtCore\/qglobal.h>/a #include <limits>' "$h"; fi
done
if ! grep -q '^#  include <limits>' "$src/src/corelib/global/qglobal.h"; then
    sed -i '/#  include <type_traits>/a #  include <limits>' "$src/src/corelib/global/qglobal.h"
fi
export PATH="$tc/bin:$PATH"
export MAKEFLAGS=-j8
cd "$sdk/configure-build"
"$src/configure" -opensource -confirm-license -release -prefix /usr \
    -extprefix "$sdk/qt5129" -hostprefix "$sdk/host" \
    -xplatform linux-lab7-g++ -nomake tests -nomake examples \
    -no-opengl -no-xcb -no-icu -no-openssl -no-dbus -linuxfb \
    > configure.log 2>&1
tail -25 configure.log
