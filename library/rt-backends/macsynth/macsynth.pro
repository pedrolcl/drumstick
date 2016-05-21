TEMPLATE = lib
CONFIG += plugin create_prl #static
TARGET = $$qtLibraryTarget(drumstick-rt-macsynth)
DESTDIR = ../../../build/lib/drumstick
DEPENDPATH += . ../../include
INCLUDEPATH += . ../../include
include (../../../global.pri)
QT -= gui

HEADERS += ../../include/rtmidioutput.h \
           macsynth.h

SOURCES += macsynth.cpp

LIBS += -framework CoreMidi -framework CoreFoundation -framework CoreServices
LIBS += -framework CoreAudio -framework AudioToolbox -framework AudioUnit
