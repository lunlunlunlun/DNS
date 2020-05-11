#-------------------------------------------------
#
# Project created by QtCreator 2020-05-07T13:46:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtclient
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

//导入winsock
LIBS += -lWS2_32

FORMS    += mainwindow.ui

RESOURCES += \
    icon.qrc
