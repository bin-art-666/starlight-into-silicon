#include "mainwindow.h"
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("第六次实验 - Qt文本读写"));
    resize(800, 480);
    textEdit = new QTextEdit(this);
    textEdit->setObjectName("textEdit");
    textEdit->setAcceptRichText(false);
    textEdit->setReadOnly(true);
    textEdit->setPlaceholderText(QStringLiteral("点击“打开”选择 UTF-8 文本，编辑后点击“关闭”保存。"));
    openPushButton = new QPushButton(QStringLiteral("打开"), this);
    closePushButton = new QPushButton(QStringLiteral("关闭"), this);
    openPushButton->setObjectName("openButton");
    closePushButton->setObjectName("closeButton");
    for (auto button : {openPushButton, closePushButton}) {
        button->setMinimumHeight(50);
        button->setMaximumWidth(120);
    }
    closePushButton->setEnabled(false);
    auto buttons = new QHBoxLayout;
    buttons->addWidget(openPushButton);
    buttons->addWidget(closePushButton);
    auto layout = new QVBoxLayout;
    layout->addWidget(textEdit);
    layout->addLayout(buttons);
    auto widget = new QWidget(this);
    widget->setLayout(layout);
    setCentralWidget(widget);
    statusBar()->showMessage(QStringLiteral("未打开文件"));
    connect(openPushButton, &QPushButton::clicked, this, &MainWindow::openFile);
    connect(closePushButton, &QPushButton::clicked, this, &MainWindow::closeFile);
}

bool MainWindow::openFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("打开 UTF-8 文本"), QString(),
        QStringLiteral("文本文件 (*.txt);;所有文件 (*)"));
    if (path.isEmpty()) return false;
    file.setFileName(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QStringLiteral("读取失败"), file.errorString());
        return false;
    }
    const QByteArray bytes = file.readAll();
    const bool success = file.error() == QFileDevice::NoError;
    const QString error = file.errorString();
    file.close();
    if (!success) {
        QMessageBox::warning(this, QStringLiteral("读取失败"), error);
        return false;
    }
    textEdit->setPlainText(QString::fromUtf8(bytes));
    textEdit->setReadOnly(false);
    openPushButton->setEnabled(false);
    closePushButton->setEnabled(true);
    statusBar()->showMessage(QStringLiteral("已打开：") + path);
    return true;
}

bool MainWindow::closeFile()
{
    if (!closePushButton->isEnabled()) return true;
    // 与教材一致，以 WriteOnly | Text 打开，覆盖写回原文件。
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QStringLiteral("保存失败"), file.errorString());
        return false;
    }
    const QByteArray bytes = textEdit->toPlainText().toUtf8();
    const qint64 written = file.write(bytes);
    const bool flushed = file.flush();
    const QString error = file.errorString();
    file.close();
    if (written != bytes.size() || !flushed) {
        QMessageBox::warning(this, QStringLiteral("保存失败"), error);
        return false;
    }
    textEdit->clear();
    textEdit->setReadOnly(true);
    openPushButton->setEnabled(true);
    closePushButton->setEnabled(false);
    statusBar()->showMessage(QStringLiteral("已保存并关闭：") + file.fileName());
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 直接关闭窗口也写回，保存失败则保留编辑内容。
    if (closeFile()) event->accept();
    else event->ignore();
}
