#-------------------------------------------------
#
# Project created by QtCreator 2020-12-16T11:08:25
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = my_FileTransfer_1
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    clientwidget.cpp

HEADERS  += widget.h \
    clientwidget.h

FORMS    += widget.ui \
    clientwidget.ui

CONFIG += C++11
