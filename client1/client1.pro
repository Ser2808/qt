#-------------------------------------------------
#
# Project created by QtCreator 2015-06-01T16:34:05
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = client1
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    clientthread.cpp

HEADERS  += mainwindow.h \
    clientthread.h

FORMS    += mainwindow.ui
