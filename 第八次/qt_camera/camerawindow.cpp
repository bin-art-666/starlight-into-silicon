#include "camerawindow.h"
#include "cameraworker.h"
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QSaveFile>
#include <QTimer>
#include <QUuid>
#include <QDebug>

CameraWindow::CameraWindow(const QString &directory,QWidget *parent)
    : QWidget(parent),saveDirectory(directory) {
    setWindowTitle(QStringLiteral("实验八 Qt控制Camera"));resize(1024,600);
    auto root=new QVBoxLayout(this);
    auto title=new QLabel(QStringLiteral("实验八  Qt摄像头预览与拍照"));
    QFont heading=font();heading.setPointSize(21);heading.setBold(true);title->setFont(heading);
    root->addWidget(title);
    auto bar=new QHBoxLayout;
    devices=new QComboBox;devices->setEditable(true);devices->setMinimumWidth(180);
    resolution=new QComboBox;resolution->addItems({"640x480","320x240"});
    refreshButton=new QPushButton(QStringLiteral("刷新设备"));
    openButton=new QPushButton(QStringLiteral("打开摄像头"));
    stopButton=new QPushButton(QStringLiteral("停止"));
    photoButton=new QPushButton(QStringLiteral("拍照保存"));
    bar->addWidget(devices,1);bar->addWidget(resolution);
    for(auto button:{refreshButton,openButton,stopButton,photoButton}) {
        button->setMinimumHeight(48);bar->addWidget(button);
    }
    root->addLayout(bar);
    preview=new QLabel(QStringLiteral("摄像头尚未开启\n请连接USB或配套CSI摄像头后选择设备"));
    preview->setAlignment(Qt::AlignCenter);preview->setMinimumSize(320,220);
    preview->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Expanding);
    preview->setStyleSheet("background:#17212b;color:#e4ebf2;font-size:20px;");
    root->addWidget(preview,1);
    message=new QLabel;message->setWordWrap(true);root->addWidget(message);
    auto path=new QLabel(QStringLiteral("照片目录：")+QDir::toNativeSeparators(saveDirectory));
    path->setWordWrap(true);root->addWidget(path);
    auto closeButton=new QPushButton(QStringLiteral("退出"));closeButton->setMinimumHeight(40);
    root->addWidget(closeButton);
    connect(refreshButton,&QPushButton::clicked,this,[this]{refreshDevices();});
    connect(openButton,&QPushButton::clicked,this,[this]{openCamera();});
    connect(stopButton,&QPushButton::clicked,this,[this]{stopCamera();});
    connect(photoButton,&QPushButton::clicked,this,[this]{savePhoto();});
    connect(closeButton,&QPushButton::clicked,this,&QWidget::close);
    setRunning(false);refreshDevices();
}

void CameraWindow::refreshDevices() {
    const QString old=devices->currentText();devices->clear();
    const auto files=QDir("/dev").entryList({"video[0-9]*"},QDir::System|QDir::Files,QDir::Name);
    for(const QString &name:files) devices->addItem("/dev/"+name);
    if(devices->count()==0) devices->addItem("/dev/video0");
    if(!old.isEmpty()) devices->setEditText(old);
    message->setText(files.isEmpty()
        ? QStringLiteral("未检测到视频设备。接口已预留，接入摄像头后点击“刷新设备”。")
        : QStringLiteral("发现视频设备节点，尚未开启；节点存在不代表摄像头已连接或可采集。"));
}

void CameraWindow::setRunning(bool active) {
    devices->setEnabled(!active);resolution->setEnabled(!active);
    refreshButton->setEnabled(!active);openButton->setEnabled(!active);
    stopButton->setEnabled(active);photoButton->setEnabled(false);
}

void CameraWindow::openCamera() {
    if(worker) return;
    currentFrame=QImage();preview->clear();preview->setText(QStringLiteral("正在连接摄像头……"));
    const QStringList size=resolution->currentText().split('x');
    worker=new CameraWorker(this,devices->currentText().trimmed(),size[0].toInt(),size[1].toInt());
    CameraWorker *session=worker;
    worker->onStatus=[this](const QString &text){message->setText(text);qInfo().noquote()<<text;};
    worker->onFrame=[this](const QImage &image){
        currentFrame=image;updatePreview();photoButton->setEnabled(true);
    };
    connect(worker,&QThread::finished,this,[this,session] {
        if(worker!=session) return;
        // queued frame/status notifications may still reference the worker;
        // deleteLater keeps it alive until previously queued deliveries finish.
        worker=nullptr;session->deleteLater();setRunning(false);
        currentFrame=QImage();preview->clear();preview->setText(QStringLiteral("预览已停止\n可重新选择设备并打开"));
        if(message->text().startsWith(QStringLiteral("正在停止")))
            message->setText(QStringLiteral("摄像头已停止，可重新打开。"));
        if(closing) QTimer::singleShot(0,this,[this]{close();});
    });
    setRunning(true);worker->start();
}

void CameraWindow::stopCamera() {
    if(!worker) return;
    photoButton->setEnabled(false);stopButton->setEnabled(false);
    worker->requestInterruption();
    message->setText(QStringLiteral("正在停止，等待摄像头驱动返回……"));
}

void CameraWindow::updatePreview() {
    if(!currentFrame.isNull()) preview->setPixmap(QPixmap::fromImage(currentFrame).scaled(
        preview->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
}

bool CameraWindow::savePhoto() {
    if(!worker || !worker->isRunning() || currentFrame.isNull()) {
        message->setText(QStringLiteral("尚无有效摄像头画面，不能拍照。"));return false;
    }
    if(!QDir().mkpath(saveDirectory)) {
        message->setText(QStringLiteral("无法创建照片目录：")+saveDirectory);return false;
    }
    const QString name=QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz")+"_"+
        QUuid::createUuid().toString(QUuid::WithoutBraces).left(8)+".png";
    const QString path=QDir(saveDirectory).filePath(name);
    QSaveFile file(path);
    if(!file.open(QIODevice::WriteOnly) || !currentFrame.save(&file,"PNG") || !file.commit()) {
        message->setText(QStringLiteral("保存失败：")+file.errorString());return false;
    }
    message->setText(QStringLiteral("照片已保存：")+path);return true;
}

void CameraWindow::resizeEvent(QResizeEvent *event) {QWidget::resizeEvent(event);updatePreview();}
void CameraWindow::closeEvent(QCloseEvent *event) {
    if(worker) {closing=true;stopCamera();event->ignore();}
    else event->accept();
}

bool CameraWindow::selfCheck(const QString &evidence) {
    if(photoButton->isEnabled() || stopButton->isEnabled() || !openButton->isEnabled()) return false;
    if(savePhoto()) return false;
    cv::Mat bgr(2,2,CV_8UC3,cv::Scalar(7,31,201));
    QImage image=CameraWorker::toImage(bgr);bgr.setTo(cv::Scalar(0,0,0));
    if(image.isNull() || image.pixelColor(0,0)!=QColor(201,31,7)) return false;
    cv::Mat gray(2,2,CV_8UC1,cv::Scalar(80));
    if(CameraWorker::toImage(gray).pixelColor(0,0)!=QColor(80,80,80)) return false;
    if(!CameraWorker::toImage(cv::Mat()).isNull()) return false;
    if(!QDir().mkpath(evidence)) return false;
    message->setText(QStringLiteral("程序接口已预留，尚未连接摄像头，未进行取景或拍照验证。"));
    if(!grab().save(evidence+"/01_interface_ready.png")) return false;
    devices->setEditText("/dev/video255"); // absence is checked by main before self-check
    openCamera();
    return true;
}
