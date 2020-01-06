TEMPLATE = lib
CONFIG += c++11 plugin
static {
    CONFIG += staticlib create_prl
}
TARGET = drumstick-rt-win-in
DESTDIR = ../../../build/lib/drumstick
DEPENDPATH += . ../../include
INCLUDEPATH += . ../../include
include (../../../global.pri)
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
QT -= gui

HEADERS += winmidiinput.h
SOURCES += winmidiinput.cpp
LIBS += -lwinmm
LIBS += -L$$OUT_PWD/../../../build/lib -l$$drumstickLib(drumstick-rt)
