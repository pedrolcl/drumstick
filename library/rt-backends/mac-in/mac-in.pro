TEMPLATE = lib
CONFIG += plugin
static {
    CONFIG += staticlib create_prl
}
TARGET = $$qtLibraryTarget(drumstick-rt-mac-in)
DESTDIR = ../../../build/lib/drumstick
DEPENDPATH += . ../../include ../common
INCLUDEPATH += . ../../include ../common
include (../../../global.pri)
QT -= gui
QT += concurrent

HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           macmidiinput.h \
           ../common/maccommon.h

SOURCES += macmidiinput.cpp \
           ../common/maccommon.cpp

LIBS += -framework CoreMidi -framework CoreFoundation

#-framework CoreAudio -framework AudioToolbox -framework AudioUnit
