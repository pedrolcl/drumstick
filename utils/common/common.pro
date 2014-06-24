TEMPLATE = lib
TARGET = $$qtLibraryTarget(drumstick-common)
DESTDIR = ../../build/lib
DEPENDPATH += . ../include ../src
INCLUDEPATH += . ../include ../../build/common
include (../../global.pri)
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += qt staticlib create_prl
# Input
HEADERS += cmdlineargs.h \
           cmdversion.h
SOURCES += cmdlineargs.cpp
