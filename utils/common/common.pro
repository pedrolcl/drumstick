TEMPLATE = lib
TARGET = $$qtLibraryTarget(drumstick-common)
DESTDIR = ../../build/lib
DEPENDPATH += . ../include ../src
INCLUDEPATH += . ../include 
include (../../global.pri)
CONFIG += qt staticlib create_prl
# Input
HEADERS += cmdlineargs.h \
           cmdversion.h
SOURCES += cmdlineargs.cpp
