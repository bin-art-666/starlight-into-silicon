#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
record="$PWD/实测记录"
mkdir -p "$record" /srv/tftp /srv/nfs/lab3 /mnt/lab3-nfs-check
backup="$record/config-backup-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$backup"
cp -a /etc/default/tftpd-hpa "$backup/"
[ ! -f /etc/exports.d/lab3.exports ] || cp -a /etc/exports.d/lab3.exports "$backup/"
cat >/etc/default/tftpd-hpa <<'EOF'
TFTP_USERNAME="tftp"
TFTP_DIRECTORY="/srv/tftp"
TFTP_ADDRESS="0.0.0.0:69"
TFTP_OPTIONS="--secure --verbose"
EOF
mkdir -p /etc/exports.d
# Initially permit local verification only; add the confirmed board IP later.
echo '/srv/nfs/lab3 127.0.0.1(rw,sync,no_subtree_check,root_squash)' >/etc/exports.d/lab3.exports
chmod 755 /srv/tftp /srv/nfs/lab3
printf 'Lab 3 TFTP transfer test\n' >/srv/tftp/tftp_test.txt
install -m 644 ../裸机LED实验/工作工程/c_500/ledc.bin /srv/tftp/ledc.bin
arm-linux-gnueabihf-gcc -static -O0 -g -Wall -Wextra hello_nfs.c -o hello_nfs_arm
install -m 755 hello_nfs_arm /srv/nfs/lab3/hello_nfs_arm
install -m 644 hello_nfs.c /srv/nfs/lab3/hello_nfs.c
modprobe nfsd
modprobe nfs
systemctl restart tftpd-hpa
systemctl restart nfs-server
exportfs -ra
cd "$record"
timeout 15 tftp 127.0.0.1 -m binary -c get tftp_test.txt tftp_download.txt
cmp /srv/tftp/tftp_test.txt tftp_download.txt
timeout 15 tftp 127.0.0.1 -m binary -c get ledc.bin ledc_download.bin
cmp /srv/tftp/ledc.bin ledc_download.bin
mount -t nfs -o vers=3,proto=tcp,nolock 127.0.0.1:/srv/nfs/lab3 /mnt/lab3-nfs-check
trap 'umount /mnt/lab3-nfs-check' EXIT
cmp /srv/nfs/lab3/hello_nfs_arm /mnt/lab3-nfs-check/hello_nfs_arm
cat /mnt/lab3-nfs-check/hello_nfs.c
file /mnt/lab3-nfs-check/hello_nfs_arm
sha256sum /srv/tftp/ledc.bin ledc_download.bin /srv/nfs/lab3/hello_nfs_arm /mnt/lab3-nfs-check/hello_nfs_arm
findmnt /mnt/lab3-nfs-check
exportfs -v
rpcinfo -p 127.0.0.1
systemctl is-active tftpd-hpa nfs-server
printf '\nPASS: local TFTP transfers and NFSv3 mount/read checks. Board execution is pending.\n'
