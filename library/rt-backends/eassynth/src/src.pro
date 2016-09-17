TEMPLATE = lib
TARGET = $$qtLibraryTarget(drumstick-rt-eassynth)
DESTDIR = ../../../../build/lib/drumstick
include (../../../../global.pri)
CONFIG += plugin create_prl #static
DEPENDPATH += ../sonivox
DEPENDPATH += ../../../include
INCLUDEPATH += ../sonivox/host_src
INCLUDEPATH += ../../../include
QT -= gui
LIBS += -L../../../../build/lib \
        -lsonivox

HEADERS += ../../../include/rtmidiinput.h \
           ../../../include/rtmidioutput.h \
           synthcontroller.h \
           synthrenderer.h

SOURCES += synthcontroller.cpp synthrenderer.cpp

CONFIG += link_pkgconfig
packagesExist(libpulse-simple) {
    PKGCONFIG += libpulse-simple
}
