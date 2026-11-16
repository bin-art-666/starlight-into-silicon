#include "cameraworker.h"
#include <QFileInfo>
#include <QRegularExpression>
#include <QMetaObject>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

CameraWorker::CameraWorker(QObject *target,const QString &path,int w,int h)
    : QThread(target),receiver(target),device(path),width(w),height(h) {}

void CameraWorker::status(const QString &message) {
    if(receiver) QMetaObject::invokeMethod(receiver.data(),[this,message] {
        if(onStatus) onStatus(message);
    },Qt::QueuedConnection);
}

QImage CameraWorker::toImage(const cv::Mat &frame) {
    if(frame.empty() || frame.depth()!=CV_8U) return QImage();
    cv::Mat rgb;
    if(frame.channels()==3) cv::cvtColor(frame,rgb,cv::COLOR_BGR2RGB);
    else if(frame.channels()==1) cv::cvtColor(frame,rgb,cv::COLOR_GRAY2RGB);
    else if(frame.channels()==4) cv::cvtColor(frame,rgb,cv::COLOR_BGRA2RGB);
    else return QImage();
    // 深拷贝：跨线程显示的图像不引用已释放的OpenCV内存。
    return QImage(rgb.data,rgb.cols,rgb.rows,int(rgb.step),QImage::Format_RGB888).copy();
}

void CameraWorker::run() {
    const auto match=QRegularExpression("^/dev/video([0-9]+)$").match(device);
    if(!match.hasMatch()) {status(QStringLiteral("设备路径应为 /dev/videoN"));return;}
    bool indexOK=false;const int index=match.captured(1).toInt(&indexOK);
    if(!indexOK || index>255) {status(QStringLiteral("设备编号超出支持范围"));return;}
    if(!QFileInfo::exists(device)) {
        status(QStringLiteral("未连接摄像头：%1 不存在，请接入后刷新设备。").arg(device));return;
    }
    try {
        cv::VideoCapture capture;
        status(QStringLiteral("正在打开 %1，请稍候……").arg(device));
        // OpenCV 3.1 的 V4L2 后端通过设备编号加后端常量指定。
        if(!capture.open(index+cv::CAP_V4L2)) {
            status(QStringLiteral("无法打开 %1：请检查摄像头、权限、驱动或设备占用。").arg(device));return;
        }
        capture.set(cv::CAP_PROP_FRAME_WIDTH,width);
        capture.set(cv::CAP_PROP_FRAME_HEIGHT,height);
        capture.set(cv::CAP_PROP_FPS,10);
        int emptyFrames=0;bool first=true;
        while(!isInterruptionRequested()) {
            cv::Mat frame;
            if(!capture.read(frame) || frame.empty()) {
                if(++emptyFrames>=5) {
                    status(QStringLiteral("图像读取失败，摄像头可能已断开或格式不兼容。"));break;
                }
                msleep(100);continue;
            }
            emptyFrames=0;
            const QImage image=toImage(frame);
            if(image.isNull()) {status(QStringLiteral("摄像头返回了不支持的图像格式。"));break;}
            if(first) {
                status(QStringLiteral("预览中：%1，实际 %2×%3").arg(device).arg(image.width()).arg(image.height()));
                first=false;
            }
            // 最多保留一个待显示帧，避免界面繁忙时累积图像。
            if(!pending.exchange(true) && receiver) {
                QMetaObject::invokeMethod(receiver.data(),[this,image] {
                    if(onFrame) onFrame(image);
                    pending=false;
                },Qt::QueuedConnection);
            }
            msleep(70);
        }
        capture.release();
    } catch(const cv::Exception &e) {
        status(QStringLiteral("摄像头处理异常：")+QString::fromUtf8(e.what()));
    } catch(const std::exception &e) {
        status(QStringLiteral("处理失败：")+QString::fromUtf8(e.what()));
    }
}
