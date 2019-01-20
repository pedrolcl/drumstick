TEMPLATE = lib
TARGET = $$qtLibraryTarget(drumstick-rt-dummy-out)
DESTDIR = ../../../build/lib/drumstick
include (../../../global.pri)
CONFIG += plugin
static {
    CONFIG += staticlib create_prl
}
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
QT -= gui

HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           dummyoutput.h

SOURCES += \
           dummyoutput.cpp

OTHER_FILES += \
    CMakeLists.txt

