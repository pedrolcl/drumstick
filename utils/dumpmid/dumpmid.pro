TEMPLATE = app
TARGET = drumstick-dumpmid
QT += dbus
CONFIG += qt console thread exceptions link_prl link_pkgconfig
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
DEPENDPATH += . ../../library ../../library/include
include (../../global.pri)
# Input
HEADERS += dumpmid.h
SOURCES += dumpmid.cpp

LIBS = -L$$OUT_PWD/../../build/lib \
    -l$$qtLibraryTarget(drumstick-common) \
    -l$$qtLibraryTarget(drumstick-alsa)

PKGCONFIG += alsa
