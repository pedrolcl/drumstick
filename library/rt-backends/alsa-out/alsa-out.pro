TEMPLATE = lib
CONFIG += plugin create_prl #static
TARGET = $$qtLibraryTarget(drumstick-rt-alsa-out)
DESTDIR = ../../../build/lib/drumstick
DEPENDPATH += . ../../include
INCLUDEPATH += . ../../include
include (../../../global.pri)
QT -= gui

HEADERS += ../../include/rtmidioutput.h \
           alsamidioutput.h
SOURCES += alsamidioutput.cpp

LIBS += -L../../../build/lib \
        -l$$qtLibraryTarget(drumstick-alsa) \
        -lasound

#CONFIG += link_pkgconfig
#PKGCONFIG += drumstick-alsa
