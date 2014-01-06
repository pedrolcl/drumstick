TEMPLATE = app
TARGET = drumstick-dumpsmf
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += qt console
DESTDIR = ../../build/bin
OBJECTS_DIR = ../../build/obj
MOC_DIR = ../../build/moc
INCLUDEPATH += . ../../library/include ../common ../../build/common
LIBS = -L../../build/lib -ldrumstick-common -ldrumstick-file
include (../../global.pri)
# Input
HEADERS += dumpsmf.h
SOURCES += dumpsmf.cpp
