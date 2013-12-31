TEMPLATE = lib
TARGET = drumstick-common
DESTDIR = ../../build/lib
OBJECTS_DIR = ../../build/obj
MOC_DIR = ../../build/moc
DEPENDPATH += . ../include ../src
INCLUDEPATH += . ../include ../../build/common
include (../global.pri)
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += qt staticlib
# Input
HEADERS += cmdlineargs.h
SOURCES += cmdlineargs.cpp
