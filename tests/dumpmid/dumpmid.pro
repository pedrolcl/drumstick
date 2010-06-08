TEMPLATE = app
TARGET = drumstick-dumpmid
DESTDIR = ../../build/bin
OBJECTS_DIR = ../../build/obj
MOC_DIR = ../../build/moc
INCLUDEPATH += . ../../library/include ../common ../../build/common
DEPENDPATH += . ../../library ../../library/include
LIBS = -L../../build/lib -ldrumstick-alsa  -lasound
# Input
HEADERS += dumpmid.h
SOURCES += dumpmid.cpp ../common/cmdlineargs.cpp
