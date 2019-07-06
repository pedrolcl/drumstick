TEMPLATE = lib
CONFIG += plugin
static {
    CONFIG += staticlib create_prl
}
TARGET = drumstick-rt-mac-out
DESTDIR = ../../../build/lib/drumstick
DEPENDPATH += . ../../include ../common
INCLUDEPATH += . ../../include ../common
include (../../../global.pri)
QT -= gui

HEADERS += ../../include/drumstick/rtmidiinput.h \
           ../../include/drumstick/rtmidioutput.h \
           macmidioutput.h \
           ../common/maccommon.h

SOURCES += \
           macmidioutput.cpp \
           ../common/maccommon.cpp

LIBS += -framework CoreMIDI -framework CoreFoundation

#-framework CoreAudio -framework AudioToolbox -framework AudioUnit
