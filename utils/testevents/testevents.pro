TEMPLATE = app
TARGET = drumstick-testevents
DESTDIR = ../../build/bin
OBJECTS_DIR = ../../build/obj
MOC_DIR = ../../build/moc
INCLUDEPATH += . ../../library/include ../common ../../build/common
LIBS = -L../../build/lib -ldrumstick-alsa  -lasound
# Input
HEADERS += testevents.h
SOURCES += testevents.cpp  ../common/cmdlineargs.cpp
