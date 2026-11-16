#pragma once
#include <QWidget>
#include <QImage>
class QComboBox;
class QPushButton;
class QLabel;
class CameraWorker;
class QCloseEvent;

class CameraWindow : public QWidget {
public:
    explicit CameraWindow(const QString &saveDirectory,QWidget *parent=nullptr);
    bool selfCheck(const QString &evidence);
protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
private:
    QComboBox *devices,*resolution;
    QPushButton *refreshButton,*openButton,*stopButton,*photoButton;
    QLabel *preview,*message;
    CameraWorker *worker=nullptr;
    QImage currentFrame;
    QString saveDirectory;
    bool closing=false;
    void refreshDevices();
    void openCamera();
    void stopCamera();
    bool savePhoto();
    void updatePreview();
    void setRunning(bool active);
};
