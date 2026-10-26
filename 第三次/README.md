# 实验三 · 星桥渡码

## 目标与方案

搭建 TFTP/NFS 实验环境：U-Boot 通过 TFTP 下载并运行裸机程序；Linux 通过 NFS 挂载共享目录并运行 ARM C 程序。最终方案由 WSL 编译、Windows 本机 tftpy 0.8.7 与 WinNFSd 2.4.0 提供服务。

实测 ENET2 接电脑网卡，电脑 `192.168.1.100/24`，开发板 `192.168.1.4/24`，串口 115200、8N1。`tools/WinNFSd.exe` 为历史服务程序，tftpy 需自行安装。

## TFTP 裸机下载

复用实验二的 `c_500`，文件 `tftp/ledc.bin` 为 588 字节。U-Boot 中执行：

```text
setenv ipaddr 192.168.1.4
setenv serverip 192.168.1.100
setenv netmask 255.255.255.0
tftp 0x87800000 ledc.bin
crc32 0x87800000 0x24c
dcache off
icache off
go 0x87800000
```

CRC32 实测为 `2658fc5a`，与本机一致；串口显示跳转，现场确认 LED 闪烁。未执行 `saveenv` 或烧写 SD/eMMC。

## NFS 运行 Linux 程序

```sh
arm-linux-gnueabihf-gcc -static -O0 -g -Wall -Wextra hello_nfs.c -o nfs/hello_nfs_arm
# 以下在开发板执行
mkdir -p /mnt/lab3
mount -t nfs -o vers=3,proto=tcp,nolock,soft,timeo=20,retrans=1 192.168.1.100:/lab3 /mnt/lab3
/mnt/lab3/hello_nfs_arm
echo $?
```

程序输出实验提示及 1—5 的平方，退出码为 0，`/proc/mounts` 确认来自 NFSv3/TCP 挂载。普通 Linux ELF 与裸机 BIN 依赖的运行环境不同。

## 排查与证据

WSL 镜像网络启动失败后恢复 NAT，服务迁至 Windows 本机；早期路由、IP 转发及 UDP 范围调整已恢复。`configure_*`、`setup_server.sh` 等属于历史排查材料，不应直接重跑。

最终依据为 `实验完成记录.md`、`实测记录/UBoot_TFTP下载.txt`、`UBoot_裸机C程序跳转.txt` 与 `开发板下载与NFS运行验证.txt`。`实验进度与接线.md` 是早期阶段记录。本实验没有做板端 GDB 单步/断点调试，也没有搭建 NFS 根文件系统。
