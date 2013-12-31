TEMPLATE = app
TARGET = drumstick-dumpmid
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += dbus
CONFIG += qt console thread exceptions
DESTDIR = ../../build/bin
OBJECTS_DIR = ../../build/obj
MOC_DIR = ../../build/moc
INCLUDEPATH += . ../../library/include ../common ../../build/common
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../build/lib -ldrumstick-common -ldrumstick-alsa  -lasound
# Input
HEADERS += dumpmid.h
SOURCES += dumpmid.cpp
