TEMPLATE = lib
CONFIG += plugin
static {
    CONFIG += staticlib create_prl
}
TARGET = drumstick-rt-macsynth
DESTDIR = ../../../build/lib/drumstick
DEPENDPATH += . ../../include
INCLUDEPATH += . ../../include
include (../../../global.pri)
QT -= gui

HEADERS += ../../include/drumstick/rtmidioutput.h \
           macsynth.h

SOURCES += macsynth.cpp

LIBS += -framework CoreMIDI -framework CoreFoundation -framework CoreServices
LIBS += -framework CoreAudio -framework AudioToolbox -framework AudioUnit
