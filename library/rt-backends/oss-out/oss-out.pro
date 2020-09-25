TEMPLATE = lib
TARGET = drumstick-rt-oss-out
DESTDIR = ../../../build/lib/drumstick2
include (../../../global.pri)
CONFIG += c++11 plugin
static {
    CONFIG += staticlib create_prl
}
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
QT -= gui

HEADERS += ossoutput.h
SOURCES += ossoutput.cpp

LIBS += -L$$OUT_PWD/../../../build/lib -ldrumstick-rt
