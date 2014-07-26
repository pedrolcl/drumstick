TEMPLATE = lib
TARGET = $$qtLibraryTarget(drumstick-rt-dummy-out)
DESTDIR = ../../../build/lib/drumstick
include (../../../global.pri)
CONFIG += plugin create_prl #static
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

