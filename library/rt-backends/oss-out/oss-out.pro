TEMPLATE = lib
TARGET = drumstick-rt-oss-out
DESTDIR = ../../../build/lib/drumstick
include (../../../global.pri)
CONFIG += plugin
static {
    CONFIG += staticlib create_prl
}
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
QT -= gui

HEADERS += ossoutput.h
SOURCES += ossoutput.cpp
