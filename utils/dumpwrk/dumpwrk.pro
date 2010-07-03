TEMPLATE = app
TARGET = drumstick-dumpwrk
DESTDIR = ../../build/bin
OBJECTS_DIR = ../../build/obj
MOC_DIR = ../../build/moc
INCLUDEPATH += . ../../library/include ../common ../../build/common
LIBS = -L../../build/lib -ldrumstick-file
# Input
HEADERS += dumpwrk.h
SOURCES += dumpwrk.cpp ../common/cmdlineargs.cpp
