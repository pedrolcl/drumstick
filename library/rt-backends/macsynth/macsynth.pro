TEMPLATE = lib
CONFIG += c++11 plugin
static {
    CONFIG += staticlib create_prl
}
TARGET = drumstick-rt-macsynth
DESTDIR = ../../../build/lib/drumstick2
DEPENDPATH += . ../../include
INCLUDEPATH += . ../../include
include (../../../global.pri)
QT -= gui

HEADERS += macsynth.h
SOURCES += macsynth.cpp

!static:LIBS += -F$$OUT_PWD/../../../build/lib -framework drumstick-rt
LIBS += -framework CoreMIDI -framework CoreFoundation -framework CoreServices
LIBS += -framework CoreAudio -framework AudioToolbox -framework AudioUnit
