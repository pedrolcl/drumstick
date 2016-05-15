TEMPLATE = lib
CONFIG += plugin #create_prl #static
TARGET = $$qtLibraryTarget(drumstick-rt-macsynth)
DESTDIR = ../../../build/lib/drumstick
DEPENDPATH += . ../../include ../common
INCLUDEPATH += . ../../include ../common
include (../../../global.pri)
QT -= gui

HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           macsynth.h \
           ../common/maccommon.h

SOURCES += \
           macsynth.cpp \
           ../common/maccommon.cpp

LIBS += -framework CoreMidi -framework CoreFoundation -framework CoreServices
LIBS += -framework CoreAudio -framework AudioToolbox -framework AudioUnit
