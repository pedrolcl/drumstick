TEMPLATE = lib
TARGET = drumstick-rt-oss
DESTDIR = ../../../build/lib
OBJECTS_DIR = ../../../build/obj
MOC_DIR = ../../../build/moc
include (../../../global.pri)
CONFIG += plugin static
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           ossinput_p.h \
           ossinput.h \
           ossoutput.h

SOURCES += ossinput.cpp \
           ossinput_p.cpp \
           ossoutput.cpp

