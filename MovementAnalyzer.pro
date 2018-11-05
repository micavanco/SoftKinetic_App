#-------------------------------------------------
#
# Project created by QtCreator 2018-05-05T16:24:19
#
#-------------------------------------------------

QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MovementAnalyzer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
        videoprocessorthread.cpp \
    depthvideoprocessorthread.cpp \
    camerathread.cpp \
    projectionprocessorthread.cpp \
    chart.cpp \
    fftchart.cpp

HEADERS += \
        mainwindow.h \
        videoprocessorthread.h \
    depthvideoprocessorthread.h \
    camerathread.h \
    projectionprocessorthread.h \
    chart.h \
    fftchart.h

FORMS += \
        mainwindow.ui

win32{
include(c:/OpenCV/opencv.pri)
include(c:/OpenCV/softkinetic.pri)
include(c:/OpenCV/fftw.pri)
}

unix: !macx {
  CONFIG += link_pkgconfig
  KGCONFIG += opencv
}

unix: macx {
   INCLUDEPATH += "/usr/local/include"
   LIBS += -L"/usr/local/lib" \
      -lopencv_world
}

RESOURCES += \
    resources.qrc

DISTFILES += \
    stylesheet.css
