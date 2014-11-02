#-------------------------------------------------
#
# Project created by QtCreator 2014-07-30T16:17:33
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = CameraWrapper
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    statemachine.cpp \
    arguments.cpp \
    network_helper.cpp

HEADERS += \
    statemachine.h \
    arguments.h \
    version.h \
    network_helper.h
