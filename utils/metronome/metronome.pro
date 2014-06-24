TEMPLATE = app
TARGET = drumstick-metronome
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += dbus
CONFIG += qt console thread exceptions link_prl
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common ../../build/common
LIBS = -L../../build/lib -ldrumstick-common -ldrumstick-alsa -lasound
PRE_TARGETDEPS += $$OUT_PWD/../../build/lib/libdrumstick-common.a
include (../../global.pri)
# Input
HEADERS += metronome.h
SOURCES += metronome.cpp
