#-------------------------------------------------
#
# Project created by QtCreator 2017-01-27T14:41:59
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    camera_c.cpp \
    util.cpp \
    mask_c.cpp


INCLUDEPATH += /usr/local/include/
HEADERS  += mainwindow.h \
    camera_c.h \
    util.h \
    mask_c.h



FORMS    += mainwindow.ui



target.path = /home/pi/qt5/test
INSTALLS += target



LIBS += -L$$PWD/../../raspi/opencv-3.2.0/build/lib/ -lopencv_core
LIBS += -L$$PWD/../../raspi/opencv-3.2.0/build/lib/ -lopencv_imgproc
LIBS += -L$$PWD/../../raspi/opencv-3.2.0/build/lib/ -lopencv_imgcodecs
LIBS += -L$$PWD/../../raspi/opencv-3.2.0/build/lib/ -lopencv_videoio


LIBS += -lv4l2
