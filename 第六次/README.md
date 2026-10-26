# 实验六 · 字落窗间

## 目标与运行环境

用 Qt 完成 UTF-8 文本打开、编辑、保存和重新读取。该实验属于桌面 Qt 编程，实际在 WSL sweet / Ubuntu 24.04、x86_64、Qt 5.15.13、GCC 13.2.0 环境验证，不是 ARM 开发板程序。

## 实现

`qt_text_io/` 包含主工程与 Qt Test 验证程序。`QFileDialog` 选择路径，`QFile::readAll()` 配合 `QString::fromUtf8()` 读取，`QTextEdit` 编辑；保存使用 `toPlainText().toUtf8()`，检查写入返回值并 flush/close。

打开后禁用“打开”、启用“关闭”；“关闭”表示保存到原文件、清空编辑区并恢复按钮状态，直接关闭窗口也会保存。未打开文件时禁止编辑。保存会覆盖所选文件，建议先使用随附 `测试.txt`。

## 构建与验证

```sh
mkdir -p build/app
cd build/app
qmake ../../qt_text_io/qt_text_io.pro
make -j4
./qt_text_io
```

自动测试可另建目录，用 `qmake ../../qt_text_io/verify.pro` 构建后执行 `QT_QPA_PLATFORM=offscreen ./verify_text_io`。Qt 开发库与 Qt Test 需先安装。原 `启动实验.ps1` 是本机 WSL 启动脚本，路径随机器调整。

## 结果与证据

Qt Test 操作真实控件和文件选择对话框，覆盖取消打开、中文路径与内容、读取、较短内容覆盖、换行、按钮状态、重新打开、空文件和直接关窗口保存。记录为 4 项通过、0 项失败，含初始化和清理。

`实测记录/Qt验证.txt`、`01_editing.png`、`02_saved.png` 和 `roundtrip_result.txt` 是第一轮证据；`重新运行截图/` 保存后续复验。截图来自 offscreen 平台的真实 QWidget 抓图，不是人工操作截图；`propagateSizeHints` 警告来自该平台插件。

`构建记录/` 保留构建日志，`bin/qt_text_io` 是依赖 WSL Qt 库的 Linux x86_64 可执行文件；`参考例程/` 保留教材原始示例及作者信息。参考 Qt 开发指南 295—301 页。
