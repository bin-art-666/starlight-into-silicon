#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
class QTextEdit;
class QPushButton;
class QCloseEvent;

// 参考开发指南第 295 页 01_qfile：打开、编辑、关闭时写回。
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
protected:
    void closeEvent(QCloseEvent *event) override;
private slots:
    bool openFile();
    bool closeFile();
private:
    QTextEdit *textEdit;
    QPushButton *openPushButton;
    QPushButton *closePushButton;
    QFile file;
};
#endif
