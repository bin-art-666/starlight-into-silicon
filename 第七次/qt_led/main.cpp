#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <QDebug>
#include <QTextStream>
#include <QPixmap>
#include <QEvent>

// 对应开发指南 15.2：读取真实 LED 状态后反转，通过 QFile 写回。
class LedWindow : public QWidget {
    QPushButton *toggle;
    QLabel *status;
    QString ledDir;
    QString originalTrigger;
    QString evidence;
    bool ready = false;
    int failures = 0;
    bool verifying = false;
    bool eventFilter(QObject *object, QEvent *event) override {
        if (verifying) {
            switch (event->type()) {
            case QEvent::MouseButtonPress: case QEvent::MouseButtonRelease:
            case QEvent::MouseButtonDblClick: case QEvent::TouchBegin:
            case QEvent::TouchUpdate: case QEvent::TouchEnd:
            case QEvent::KeyPress: case QEvent::KeyRelease:
                return true;
            default: break;
            }
        }
        return QWidget::eventFilter(object,event);
    }
    bool writeValue(const QString &name, const QByteArray &value) {
        QFile f(ledDir + "/" + name);
        if (!f.open(QIODevice::WriteOnly)) return error(f.errorString());
        const qint64 n = f.write(value);
        const bool ok = f.flush();
        if (n != value.size() || !ok) return error(f.errorString());
        return true;
    }
    bool error(const QString &message) {
        status->setText(QStringLiteral("操作失败：") + message);
        qWarning().noquote() << "LED_ERROR" << message;
        return false;
    }
    bool readState(int &value) {
        QFile f(ledDir + "/brightness");
        if (!f.open(QIODevice::ReadOnly)) return error(f.errorString());
        bool ok = false;
        value = f.readAll().trimmed().toInt(&ok);
        if (!ok) return error(QStringLiteral("LED状态格式不正确"));
        value = value > 0 ? 1 : 0;
        return true;
    }
    bool refresh() {
        int value;
        if (!readState(value)) return false;
        toggle->setText(value ? QStringLiteral("LED已点亮  点击熄灭")
                              : QStringLiteral("LED已熄灭  点击点亮"));
        status->setText(QStringLiteral("真实设备 brightness = %1").arg(value));
        return true;
    }
    void verifyStep(int expected, const QString &image) {
        toggle->click();
        int actual = -1;
        const bool ok = readState(actual) && actual == expected;
        if (!ok) ++failures;
        qInfo().noquote() << "LED_VERIFY expected=" << expected << "actual=" << actual
                          << (ok ? "PASS" : "FAIL");
        QTimer::singleShot(250, this, [this,image] {
            if (!evidence.isEmpty() && !grab().save(evidence + "/" + image)) {
                ++failures;
                qWarning() << "SCREENSHOT_FAILED" << image;
            }
        });
    }
public:
    explicit LedWindow(const QString &directory, const QString &out, QWidget *parent=nullptr)
        : QWidget(parent), ledDir(directory), evidence(out) {
        setWindowTitle(QStringLiteral("实验七 Qt控制LED"));
        resize(1024,600);
        auto layout = new QVBoxLayout(this);
        auto title = new QLabel(QStringLiteral("实验七  Qt控制LED"), this);
        title->setAlignment(Qt::AlignCenter);
        QFont titleFont = title->font(); titleFont.setPointSize(26); titleFont.setBold(true);
        title->setFont(titleFont);
        toggle = new QPushButton(QStringLiteral("正在检测LED"), this);
        toggle->setObjectName("toggleLed");
        toggle->setMinimumSize(500,110);
        QFont buttonFont=toggle->font();buttonFont.setPointSize(22);toggle->setFont(buttonFont);
        status = new QLabel(this);status->setAlignment(Qt::AlignCenter);
        QFont bodyFont=status->font();bodyFont.setPointSize(18);status->setFont(bodyFont);
        auto path = new QLabel(ledDir + "/brightness", this);path->setAlignment(Qt::AlignCenter);
        auto close = new QPushButton(QStringLiteral("退出并恢复原LED触发模式"),this);
        close->setMinimumHeight(60);
        layout->addWidget(title);layout->addStretch();
        layout->addWidget(toggle,0,Qt::AlignCenter);layout->addWidget(status);
        layout->addWidget(path);layout->addStretch();layout->addWidget(close);
        connect(close,&QPushButton::clicked,this,&QWidget::close);
        connect(toggle,&QPushButton::clicked,this,[this] {
            int oldState;
            if (!ready || !readState(oldState)) return;
            if (!writeValue("brightness",oldState ? "0\n" : "1\n")) return;
            if (!refresh()) return;
            qInfo() << "LED_TOGGLED" << oldState << "->" << (oldState ? 0 : 1);
        });
        QFile trigger(ledDir + "/trigger");
        if (trigger.open(QIODevice::ReadOnly)) {
            QByteArray all=trigger.readAll();int begin=all.indexOf('[');int end=all.indexOf(']');
            if(begin>=0 && end>begin) originalTrigger=QString::fromLatin1(all.mid(begin+1,end-begin-1));
        }
        if(originalTrigger.isEmpty()) {
            error(QStringLiteral("无法读取原触发模式"));toggle->setEnabled(false);return;
        }
        ready=writeValue("trigger","none\n") && refresh();
        toggle->setEnabled(ready);
        qInfo().noquote() << "QT_VERSION" << qVersion() << "LED_DEVICE" << ledDir
                         << "ORIGINAL_TRIGGER" << originalTrigger << "READY" << ready;
    }
    ~LedWindow() override {
        if (!originalTrigger.isEmpty()) {
            bool ok=writeValue("trigger",originalTrigger.toLatin1()+"\n");
            qInfo() << "RESTORE_TRIGGER" << originalTrigger << ok;
        }
    }
    void verify() {
        if(!ready) {qCritical()<<"VERIFY_NOT_READY";QApplication::exit(2);return;}
        if(!evidence.isEmpty() && !QDir().mkpath(evidence)) {
            qCritical()<<"EVIDENCE_DIR_FAILED";QApplication::exit(2);return;
        }
        if(!writeValue("brightness","0\n")) {QApplication::exit(2);return;}
        verifying=true;
        qApp->installEventFilter(this);
        refresh();
        QTimer::singleShot(1000,this,[this]{verifyStep(1,"01_LED_on.png");});
        QTimer::singleShot(4000,this,[this]{verifyStep(0,"02_LED_off.png");});
        QTimer::singleShot(7000,this,[this]{verifyStep(1,"03_LED_on_again.png");});
        QTimer::singleShot(8000,this,[this]{
            verifying=false;
            qApp->removeEventFilter(this);
            qInfo()<<"LED_VERIFY_COMPLETE"<<"failures="<<failures;
            if (failures) QApplication::exit(3);
        });
    }
};

int main(int argc,char **argv) {
    QApplication app(argc,argv);
    QString device="/sys/class/leds/sys-led", evidence;
    const QStringList args=app.arguments();
    for (const QString &arg:args) {
        if(arg.startsWith("--device=")) device=arg.mid(9);
        if(arg.startsWith("--evidence=")) evidence=arg.mid(11);
    }
    LedWindow window(device,evidence);
    window.showFullScreen();
    if(args.contains("--verify")) QTimer::singleShot(0,&window,[&window]{window.verify();});
    return app.exec();
}
