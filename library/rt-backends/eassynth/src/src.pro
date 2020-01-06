TEMPLATE = lib
TARGET = drumstick-rt-eassynth
DESTDIR = ../../../../build/lib/drumstick
include (../../../../global.pri)
CONFIG += c++11 plugin link_prl
static {
    CONFIG += staticlib create_prl
}
DEPENDPATH += ../sonivox
DEPENDPATH += ../../../include
INCLUDEPATH += ../sonivox/host_src
INCLUDEPATH += ../../../include
QT -= gui
LIBS += -L../../../../build/lib \
        -ldrumstick-rt \
        -lsonivox

HEADERS += synthcontroller.h \
           synthrenderer.h

SOURCES += synthcontroller.cpp synthrenderer.cpp

CONFIG += link_pkgconfig
packagesExist(libpulse-simple) {
    PKGCONFIG += libpulse-simple
}
