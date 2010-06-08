TEMPLATE = app
TARGET = drumstick-dumpsmf
DESTDIR = ../../build/bin
OBJECTS_DIR = ../../build/obj
MOC_DIR = ../../build/moc
INCLUDEPATH += . ../../library/include ../common ../../build/common
LIBS = -L../../build/lib -ldrumstick-file
# Input
HEADERS += dumpsmf.h
SOURCES += dumpsmf.cpp ../common/cmdlineargs.cpp
