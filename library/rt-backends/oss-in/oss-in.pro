TEMPLATE = lib
TARGET = drumstick-rt-oss-in
DESTDIR = ../../../build/lib/drumstick2
include (../../../global.pri)
CONFIG += c++11 plugin
static {
    CONFIG += staticlib create_prl
}
DEPENDPATH += ../../include ../common
INCLUDEPATH += ../../include ../common
QT -= gui

HEADERS += ../common/midiparser.h \
           ossinput_p.h \
           ossinput.h

SOURCES += ossinput.cpp \
           ossinput_p.cpp \
           ../common/midiparser.cpp

LIBS += -L$$OUT_PWD/../../../build/lib -ldrumstick-rt
