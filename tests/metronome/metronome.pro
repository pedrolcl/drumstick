TEMPLATE = app
TARGET = drumstick-metronome
DESTDIR = ../../build/bin
OBJECTS_DIR = ../../build/obj
MOC_DIR = ../../build/moc
INCLUDEPATH += . ../../library/include ../common ../../build/common
LIBS = -L../../build/lib -ldrumstick-alsa  -lasound
# Input
HEADERS += metronome.h
SOURCES += metronome.cpp ../common/cmdlineargs.cpp
