#-------------------------------------------------
#
# Project created by QtCreator 2015-04-28T10:32:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MacGUI
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    streamthread.cpp

HEADERS  += mainwindow.h \
    streamthread.h

FORMS    += mainwindow.ui
