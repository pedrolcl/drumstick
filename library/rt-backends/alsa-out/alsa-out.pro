TEMPLATE = lib
CONFIG += plugin create_prl #static
TARGET = $$qtLibraryTarget(drumstick-rt-alsa-out)
DESTDIR = ../../../build/backends
DEPENDPATH += . ../../include
INCLUDEPATH += . ../../include
include (../../../global.pri)
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           alsamidioutput.h
SOURCES += \
           alsamidioutput.cpp

LIBS += -L../../../build/lib \
        -l$$qtLibraryTarget(drumstick-alsa) \
        -lasound

#CONFIG += link_pkgconfig
#PKGCONFIG += drumstick-alsa
