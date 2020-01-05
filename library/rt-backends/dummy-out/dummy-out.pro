TEMPLATE = lib
TARGET = drumstick-rt-dummy-out
DESTDIR = ../../../build/lib/drumstick
include (../../../global.pri)
CONFIG += plugin
static {
    CONFIG += staticlib create_prl
}
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
QT -= gui

HEADERS += dummyoutput.h
SOURCES += dummyoutput.cpp
LIBS += -L$$OUT_PWD/../../../build/lib -l$$drumstickLib(drumstick-rt)
