TEMPLATE = lib
CONFIG += plugin
static {
    CONFIG += staticlib create_prl
}
TARGET = drumstick-rt-net-in
DESTDIR = ../../../build/lib/drumstick
DEPENDPATH += . ../../include ../common
INCLUDEPATH += . ../../include ../common
include (../../../global.pri)
QT -= gui

HEADERS += ../common/midiparser.h \
           netmidiinput.h \
           netmidiinput_p.h

SOURCES += netmidiinput.cpp \
           netmidiinput_p.cpp \
           ../common/midiparser.cpp

QT += network
LIBS += -L$$OUT_PWD/../../../build/lib -l$$drumstickLib(drumstick-rt)
