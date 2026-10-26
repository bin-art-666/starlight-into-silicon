#include "mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QTextEdit>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTimer>
#include <QTest>
#include <QDir>
#include <QFileInfo>
#include <QLineEdit>

class Verify : public QObject
{
    Q_OBJECT
private:
    void selectFile(MainWindow &window, const QString &path) {
        QTimer::singleShot(100, [path] {
            auto dialog = qobject_cast<QFileDialog *>(QApplication::activeModalWidget());
            if (dialog) {
                dialog->setDirectory(QFileInfo(path).absolutePath());
                dialog->findChild<QLineEdit *>("fileNameEdit")->setText(path);
                QTimer::singleShot(500, dialog, [dialog] {
                    QMetaObject::invokeMethod(dialog, "accept", Qt::QueuedConnection);
                });
            }
        });
        window.findChild<QPushButton *>("openButton")->click();
    }
private slots:
    void cancelOpen() {
        MainWindow window;
        QTimer::singleShot(100, [] {
            if (auto dialog = qobject_cast<QFileDialog *>(QApplication::activeModalWidget()))
                dialog->reject();
        });
        window.findChild<QPushButton *>("openButton")->click();
        QVERIFY(window.findChild<QPushButton *>("openButton")->isEnabled());
        QVERIFY(!window.findChild<QPushButton *>("closeButton")->isEnabled());
    }
    void roundTrip() {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString evidence = qEnvironmentVariable("LAB6_EVIDENCE_DIR");
        QString path = evidence.isEmpty() ? dir.filePath(QStringLiteral("中文测试.txt"))
                                         : evidence + QStringLiteral("/截图验证.txt");
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        const QByteArray initial = QStringLiteral("原始中文\n需要被截断的旧内容123456789\n").toUtf8();
        QCOMPARE(file.write(initial), qint64(initial.size()));
        file.close();
        MainWindow window;
        window.show();
        selectFile(window, path);
        auto editor = window.findChild<QTextEdit *>("textEdit");
        QCOMPARE(editor->toPlainText(), QString::fromUtf8(initial));
        QVERIFY(!window.findChild<QPushButton *>("openButton")->isEnabled());
        QVERIFY(window.findChild<QPushButton *>("closeButton")->isEnabled());
        QTest::qWait(250);
        if (!evidence.isEmpty())
            QVERIFY(window.grab().save(evidence + "/00_opened.png"));
        const QString edited = QStringLiteral("第六次：Qt文本读写实验\n使用 QFile 读取和写入 UTF-8 文本。\n新增内容：Qt文本读写实验完成。\n保存后再次打开，验证内容一致。\n");
        editor->setPlainText(edited);
        QTest::qWait(250);
        if (!evidence.isEmpty()) {
            QVERIFY(window.grab().save(evidence + "/01_editing.png"));
        }
        window.findChild<QPushButton *>("closeButton")->click();
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.readAll(), edited.toUtf8());
        file.close();
        QVERIFY(editor->toPlainText().isEmpty());
        QVERIFY(editor->isReadOnly());
        QVERIFY(window.findChild<QPushButton *>("openButton")->isEnabled());
        QTest::qWait(250);
        if (!evidence.isEmpty()) {
            QVERIFY(window.grab().save(evidence + "/02_saved.png"));
            QVERIFY(QFile::copy(path, evidence + "/roundtrip_result.txt"));
        }
        // 再次打开，确认磁盘结果可回读；测试保存空文本及窗口关闭写回。
        selectFile(window, path);
        QCOMPARE(editor->toPlainText(), edited);
        QTest::qWait(250);
        if (!evidence.isEmpty())
            QVERIFY(window.grab().save(evidence + "/03_reopened.png"));
        editor->clear();
        QVERIFY(window.close());
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.readAll(), QByteArray());
        file.close();
        // 空文本测试完成后，恢复本次截图对应内容以供人工核对。
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(edited.toUtf8()), qint64(edited.toUtf8().size()));
        file.close();
    }
};

int main(int argc, char **argv) {
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    QApplication app(argc, argv);
    Verify test;
    return QTest::qExec(&test, argc, argv);
}
#include "verify.moc"
