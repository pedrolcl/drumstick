TEMPLATE = lib
TARGET = drumstick-rt-dummy-out
DESTDIR = ../../../build/lib/drumstick2
include (../../../global.pri)
CONFIG += c++11 plugin
static {
    CONFIG += staticlib create_prl
}
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
QT -= gui

HEADERS += dummyoutput.h
SOURCES += dummyoutput.cpp
LIBS += -L$$OUT_PWD/../../../build/lib -l$$drumstickLib(drumstick-rt)
