TEMPLATE = lib
TARGET = drumstick-rt-eassynth
DESTDIR = ../../../../build/lib/drumstick2
include (../../../../global.pri)
CONFIG += c++11 plugin link_prl
static {
    CONFIG += staticlib create_prl
}
DEPENDPATH += ../../../include
INCLUDEPATH += ../../../include
QT -= gui
LIBS += -L../../../../build/lib \
        -ldrumstick-rt

HEADERS += synthcontroller.h \
           synthrenderer.h

SOURCES += synthcontroller.cpp synthrenderer.cpp

CONFIG += link_pkgconfig
packagesExist(libpulse-simple) {
    PKGCONFIG += libpulse-simple
}
packagesExist(sonivox) {
    PKGCONFIG += sonivox
}
