#include "camerawindow.h"
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <QPushButton>
#include <QDebug>

int main(int argc,char **argv) {
    QApplication app(argc,argv);
    QString output=QDir::homePath()+"/lab8/photos",evidence;
    for(const QString &arg:app.arguments()) {
        if(arg.startsWith("--output=")) output=arg.mid(9);
        if(arg.startsWith("--self-check=")) evidence=arg.mid(13);
    }
    CameraWindow window(output);
#if defined(__arm__)
    window.showFullScreen();
#else
    window.show();
#endif
    if(!evidence.isEmpty()) {
        if(QFileInfo::exists("/dev/video255")) return 2;
        QTimer::singleShot(250,&window,[&window,evidence] {
            if(!window.selfCheck(evidence)) {qCritical()<<"SELF_CHECK_FAIL";QApplication::exit(2);return;}
            QTimer::singleShot(1000,&window,[&window,evidence] {
                const auto buttons=window.findChildren<QPushButton *>();
                bool open=false,photo=false,stop=false;
                for(auto b:buttons) {
                    if(b->text()==QStringLiteral("打开摄像头")) open=b->isEnabled();
                    if(b->text()==QStringLiteral("拍照保存")) photo=b->isEnabled();
                    if(b->text()==QStringLiteral("停止")) stop=b->isEnabled();
                }
                const bool saved=window.grab().save(evidence+"/02_missing_device.png");
                const bool ok=open && !photo && !stop && saved;
                qInfo()<<"SELF_CHECK"<<(ok?"PASS":"FAIL")
                       <<"checks: initial states, no-frame guard, BGR conversion, gray conversion,"
                       <<"image ownership, empty frame, missing device, UI recovery, screenshots";
                QApplication::exit(ok?0:3);
            });
        });
    }
    return app.exec();
}
