TEMPLATE = lib
TARGET = drumstick-common
DESTDIR = ../../build/lib
OBJECTS_DIR = ../../build/obj
MOC_DIR = ../../build/moc
DEPENDPATH += . ../include ../src
INCLUDEPATH += . ../include ../../build/common
include (../global.pri)
CONFIG += qt staticlib debug
# Input
HEADERS += cmdlineargs.h
SOURCES += cmdlineargs.cpp
