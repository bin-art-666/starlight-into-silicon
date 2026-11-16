QT += core gui widgets
CONFIG += c++11
TARGET = qt_camera
SOURCES += main.cpp cameraworker.cpp camerawindow.cpp
HEADERS += cameraworker.h camerawindow.h
isEmpty(OPENCV_INCLUDE_DIRS): error(Set OPENCV_INCLUDE_DIRS to matching OpenCV headers)
INCLUDEPATH += $$OPENCV_INCLUDE_DIRS
!isEmpty(OPENCV_LIBDIR): LIBS += -L$$OPENCV_LIBDIR
LIBS += -lopencv_videoio -lopencv_imgproc -lopencv_core
