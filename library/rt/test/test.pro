TEMPLATE = app
TARGET = drumstick-rt-test
CONFIG += qt console
VERSION = 0.0.1
DESTDIR = ../../../build/bin
OBJECTS_DIR = ../../../build/obj
MOC_DIR = ../../../build/moc
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
SOURCES += main.cpp
dummy {
    DEFINES += DUMMY_BACKEND
    LIBS += -L../../../build/lib -ldrumstick-rt-dummy
}
linux* {
    DEFINES += ALSA_BACKEND
    LIBS += -L../../../build/lib -ldrumstick-rt-alsa -ldrumstick-alsa -lasound
}
macx {
    DEFINES += MAC_BACKEND
    LIBS += -L../../../build/lib -ldrumstick-rt-mac -framework CoreMidi -framework CoreFoundation
}
win32 {
    DEFINES += WIN_BACKEND
    LIBS += -L../../../build/lib -ldrumstick-rt-win -lwinmm
}
DEFINES += NET_BACKEND
LIBS += -L../../../build/lib -ldrumstick-rt-net
QT += network
