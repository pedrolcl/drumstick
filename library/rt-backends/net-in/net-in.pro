TEMPLATE = lib
CONFIG += plugin #create_prl static
TARGET = $$qtLibraryTarget(drumstick-rt-net-in)
DESTDIR = ../../../build/lib/drumstick
DEPENDPATH += . ../../include ../common
INCLUDEPATH += . ../../include ../common
include (../../../global.pri)
QT -= gui

HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           ../common/midiparser.h \
           netmidiinput.h \
           netmidiinput_p.h

SOURCES += netmidiinput.cpp \
           netmidiinput_p.cpp \
           ../common/midiparser.cpp

QT += network

win32 {
    TARGET_EXT = .dll
}
