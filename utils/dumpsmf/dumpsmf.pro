TEMPLATE = app
TARGET = drumstick-dumpsmf
CONFIG += qt console
DESTDIR = ../../build/bin
OBJECTS_DIR = ../../build/obj
MOC_DIR = ../../build/moc
INCLUDEPATH += . ../../library/include ../common ../../build/common
LIBS = -L../../build/lib -ldrumstick-common -ldrumstick-file
# Input
HEADERS += dumpsmf.h
SOURCES += dumpsmf.cpp
