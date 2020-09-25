TEMPLATE = lib
CONFIG += c++11 plugin
static {
    CONFIG += staticlib create_prl
}
TARGET = drumstick-rt-net-in
DESTDIR = ../../../build/lib/drumstick2
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
macx:!static:LIBS += -F$$OUT_PWD/../../../build/lib -framework drumstick-rt
else:LIBS += -L$$OUT_PWD/../../../build/lib -l$$drumstickLib(drumstick-rt)
