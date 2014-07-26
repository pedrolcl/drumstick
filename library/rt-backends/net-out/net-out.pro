TEMPLATE = lib
CONFIG += plugin #create_prl static
TARGET = $$qtLibraryTarget(drumstick-rt-net-out)
DESTDIR = ../../../build/lib/drumstick
DEPENDPATH += . ../../include
INCLUDEPATH += . ../../include
include (../../../global.pri)
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
QT -= gui

HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           netmidioutput.h
SOURCES += \
           netmidioutput.cpp

QT += network

win32 {
    TARGET_EXT = .dll
}
