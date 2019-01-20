TEMPLATE = lib
TARGET = drumstick-rt-oss-in
DESTDIR = ../../../build/lib/drumstick
include (../../../global.pri)
CONFIG += plugin
static {
    CONFIG += staticlib create_prl
}
DEPENDPATH += ../../include ../common
INCLUDEPATH += ../../include ../common
QT -= gui

HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           ../common/midiparser.h \
           ossinput_p.h \
           ossinput.h

SOURCES += ossinput.cpp \
           ossinput_p.cpp \
           ../common/midiparser.cpp


