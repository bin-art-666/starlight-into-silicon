#!/bin/bash
set -euo pipefail
sdk=/home/little/lab7-sdk
lab=/mnt/d/Develop/Linux/嵌入式/第七次
tc=$sdk/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf
export PATH="$tc/bin:$PATH"
if [ -f /mnt/d/Develop/Linux/嵌入式/第三次/nfs/lab7/board-extra.tar.gz ]; then
    tar -xzf /mnt/d/Develop/Linux/嵌入式/第三次/nfs/lab7/board-extra.tar.gz -C "$sdk/board-sysroot"
fi
# 板端 GNU ld 脚本含绝对路径，在主机 sysroot 副本中重定位。
for name in libc.so libpthread.so; do
    sed -i -e "s| /lib/| $sdk/board-sysroot/lib/|g" \
           -e "s| /usr/lib/| $sdk/board-sysroot/usr/lib/|g" \
           "$sdk/board-sysroot/usr/lib/$name"
done
for module in Core Gui Widgets; do
    ln -sf "$sdk/board-sysroot/usr/lib/libQt5$module.so.5.12.9" "$sdk/configure-build/lib/libQt5$module.so"
done
mkdir -p "$lab/build/arm" "$lab/bin"
cd "$lab/build/arm"
"$sdk/configure-build/bin/qmake" "$lab/qt_led/qt_led.pro" \
    "QMAKE_LIBDIR+=$sdk/board-sysroot/usr/lib $sdk/board-sysroot/lib" \
    "QMAKE_LFLAGS+=-Wl,--sysroot=$sdk/board-sysroot -Wl,-rpath-link,$sdk/board-sysroot/usr/lib -Wl,-rpath-link,$sdk/board-sysroot/lib" \
    'QMAKE_RPATHDIR='
make -j4 > build.log 2>&1
cp qt_led "$lab/bin/qt_led_arm"
cp qt_led /mnt/d/Develop/Linux/嵌入式/第三次/nfs/lab7/qt_led_arm
file qt_led
arm-linux-gnueabihf-readelf --version-info qt_led > "$lab/实测记录/ARM符号版本.txt"
