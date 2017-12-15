#-------------------------------------------------
#
# Project created by QtCreator 2017-12-11T15:16:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = hourglass_monitor
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qctrlpanel.cpp \
    wsvr_associate.cpp \
    ts_associate.cpp

HEADERS  += mainwindow.h \
    debug.hpp \
    qctrlpanel.h

FORMS    += mainwindow.ui
