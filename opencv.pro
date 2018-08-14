#-------------------------------------------------
#
# Project created by QtCreator 2015-03-25T18:15:50
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++11
TARGET = opencv
TEMPLATE = app
DESTDIR = $$PWD
INCLUDEPATH += /usr/local/include/opencv \
    ./resources

# -lopencv_cudaimgproc -lopencv_cudaobjdetect

LIBS += -L/usr/local/lib -I/usr/local/include/opencv -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_objdetect -lopencv_video -lopencv_videoio -lopencv_videostab

SOURCES += main.cpp \
    gui/mainwindow.cpp \
    camera.cpp \
    gui/displaywidget.cpp \
    faceDetector.cpp \
    gui/imageviewer.cpp

HEADERS += gui/mainwindow.h \
    camera.h \
    gui/displaywidget.h \
    faceDetector.h \
    gui/imageviewer.h
