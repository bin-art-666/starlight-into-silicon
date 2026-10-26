# 实验七 · 指尖星火

## 目标与环境

将 Qt Widgets 程序交叉编译为 ARM Linux 应用，在真实 I.MX6ULL ALPHA 的 framebuffer 上运行，通过按钮控制实体 LED。实测系统为 Linux `4.1.15-g3dc0a4b`、Qt 5.12.9、glibc 2.23；显示为 linuxfb、1024×600，触摸为 goodix-ts。

## 实现与编译

`qt_led/main.cpp` 通过 `/sys/class/leds/sys-led/brightness` 读写 LED。启动时记录原触发模式（本次为 heartbeat），临时切为 none；按钮先读取当前状态再切换，写后读回；正常退出恢复原触发模式。

实际使用 Linaro GCC 5.5、Qt 5.12.9 源码生成的主机工具，以及从本板复制的既有 Qt 库组成链接 sysroot。没有安装或宣称使用 Poky SDK，也没有替换板端 Qt/libc。主机 GCC 13 编译旧 Qt 工具所需的 limits 头修补和链接脚本路径调整见 `tools/`。

脚本保留了 `/home/little/lab7-sdk` 和原实验路径，迁移前需配置；所需大归档下载与 SHA-256 见根目录 `资料索引.md`。输出程序 `bin/qt_led_arm` 为 ARM 32 位 EABI5 动态链接 ELF。

## 板端运行

```sh
cd /home/root/lab7
sh ./run_on_board.sh
# 自动执行按钮亮、灭、亮并读回状态
sh ./run_on_board.sh --verify
```

两种方式择一运行，避免重复启动。脚本暂时暂停出厂桌面绘制，程序退出后恢复。自动验证期间忽略触摸，结束后恢复手动操作。

## 结果与证据

最终亮、灭、亮三次读回分别为 1、0、1，全部 PASS，`failures=0`；现场确认实体 LED 随按钮亮灭。以 `实测记录/Qt控制LED最终验证.txt` 和 `最终板端截图/` 为准。截图是板端进程抓取的真实应用窗口，不是实体开发板照片。

首次自动验证受到额外输入干扰，失败记录保留在 `Qt控制LED板端验证.txt` 与 `板端截图/`；修正验证期输入处理后完成复验。NFS 超时与 stale handle 通过重启同一服务并重新挂载解决。

`实验完成记录.md` 记载更完整的当时环境与传输过程；其末尾“第四、第五次仍跳过”属于当时安排，整个七实验集合中这两项后来已有完成记录。
