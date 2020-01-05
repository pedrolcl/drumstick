TEMPLATE = lib
CONFIG += plugin
static {
    CONFIG += staticlib create_prl
}
TARGET = drumstick-rt-mac-in
DESTDIR = ../../../build/lib/drumstick
DEPENDPATH += . ../../include ../common
INCLUDEPATH += . ../../include ../common
include (../../../global.pri)
QT -= gui
QT += concurrent

HEADERS += macmidiinput.h \
           ../common/maccommon.h

SOURCES += macmidiinput.cpp \
           ../common/maccommon.cpp

!static:LIBS += -F$$OUT_PWD/../../../build/lib -framework drumstick-rt
LIBS += -framework CoreMIDI -framework CoreFoundation
