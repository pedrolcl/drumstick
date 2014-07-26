TEMPLATE = lib
TARGET = $$qtLibraryTarget(drumstick-rt-synth)
DESTDIR = ../../../build/lib/drumstick
include (../../../global.pri)
CONFIG += plugin #create_prl static
CONFIG += link_pkgconfig
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
QT -= gui

HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           synthengine.h \
           synthoutput.h

SOURCES += synthoutput.cpp synthengine.cpp

PKGCONFIG += fluidsynth

win32 {
    TARGET_EXT = .dll
}
