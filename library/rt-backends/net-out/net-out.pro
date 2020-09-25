TEMPLATE = lib
CONFIG += c++11 plugin
static {
    CONFIG += staticlib create_prl
}
TARGET = drumstick-rt-net-out
DESTDIR = ../../../build/lib/drumstick2
DEPENDPATH += . ../../include
INCLUDEPATH += . ../../include
include (../../../global.pri)
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
QT -= gui

HEADERS += netmidioutput.h
SOURCES += netmidioutput.cpp

QT += network
macx:!static:LIBS += -F$$OUT_PWD/../../../build/lib -framework drumstick-rt
else:LIBS += -L$$OUT_PWD/../../../build/lib -l$$drumstickLib(drumstick-rt)
