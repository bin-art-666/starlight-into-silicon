#!/bin/bash
set -euo pipefail
LAB_DIR="$(cd -- "$(dirname -- "$0")" && pwd)"
KERNELDIR="${KERNELDIR:-/tmp/lab4-kernel}"
CROSS_COMPILE="${CROSS_COMPILE:-/home/little/lab7-sdk/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-}"
command -v "${CROSS_COMPILE}gcc" >/dev/null
mkdir -p "$KERNELDIR" "$LAB_DIR/实测记录"
if [ ! -f "$KERNELDIR/Makefile" ]; then
    tar -xjf "$LAB_DIR/linux-imx-4.1.15-2.1.0-g3dc0a4b-v2.7.tar.bz2" -C "$KERNELDIR"
fi
gzip -dc "$LAB_DIR/实测记录/board-config.gz" > "$KERNELDIR/.config"
MAKE_ARGS=(ARCH=arm "CROSS_COMPILE=$CROSS_COMPILE" LOCALVERSION=-g3dc0a4b 'HOSTCFLAGS=-O2 -fcommon')
make -C "$KERNELDIR" "${MAKE_ARGS[@]}" olddefconfig
make -C "$KERNELDIR" "${MAKE_ARGS[@]}" -j8 vmlinux
make -C "$LAB_DIR/06_beep" KERNELDIR="$KERNELDIR" CROSS_COMPILE="$CROSS_COMPILE" module
# The user program is static so it does not depend on the board's old glibc.
make -C "$LAB_DIR/06_beep" CROSS_COMPILE=arm-linux-gnueabihf- app
"$KERNELDIR/scripts/dtc/dtc" -I dts -O dtb "$LAB_DIR/设备树/lab4-beep.dts" -o "$LAB_DIR/设备树/lab4-beep.dtb"
cp "$KERNELDIR/Module.symvers" "$LAB_DIR/实测记录/kernel-Module.symvers"
file "$LAB_DIR/06_beep/beep.ko" "$LAB_DIR/06_beep/beepApp"
