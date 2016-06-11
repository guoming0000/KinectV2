#-------------------------------------------------
#
# Project created by QtCreator 2016-06-11T19:20:19
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = KinectV2InQt
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp

HEADERS  += widget.h \
    inc/Kinect.Face.h \
    inc/Kinect.h \
    inc/Kinect.INPC.h \
    inc/Kinect.VisualGestureBuilder.h \
    inc/NuiKinectFusionApi.h \
    inc/NuiKinectFusionBase.h \
    inc/NuiKinectFusionCameraPoseFinder.h \
    inc/NuiKinectFusionColorVolume.h \
    inc/NuiKinectFusionDepthProcessor.h \
    inc/NuiKinectFusionVolume.h

FORMS    += widget.ui


win32: LIBS += -L$$PWD/inc/ -lKinect20

INCLUDEPATH += $$PWD/inc
DEPENDPATH += $$PWD/inc
