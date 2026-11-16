#pragma once
#include <QThread>
#include <QImage>
#include <QPointer>
#include <atomic>
#include <functional>
#include <opencv2/core.hpp>

// 预留Linux摄像头设备接口，USB/CSI均由板端驱动提供 /dev/videoN。
class CameraWorker : public QThread {
public:
    CameraWorker(QObject *receiver, const QString &device, int width, int height);
    std::function<void(const QImage &)> onFrame;
    std::function<void(const QString &)> onStatus;
    static QImage toImage(const cv::Mat &frame);
protected:
    void run() override;
private:
    QPointer<QObject> receiver;
    QString device;
    int width, height;
    std::atomic<bool> pending{false};
    void status(const QString &message);
};
