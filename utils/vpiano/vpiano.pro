TEMPLATE = app
TARGET = drumstick-vpiano
QT += svg dbus network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets gui
CONFIG += qt thread exceptions
DESTDIR = ../../build/bin
OBJECTS_DIR = ../../build/obj
MOC_DIR = ../../build/moc
RCC_DIR = ../../build/rcc
UI_DIR = ../../build/ui
include (../../global.pri)
INCLUDEPATH += . ../../library/include ../common ../../build/common
LIBS += -L../../build/lib -ldrumstick-common -ldrumstick-rt-net
linux* {
    LIBS += -L../../build/lib -ldrumstick-rt-alsa -ldrumstick-alsa -lasound
}
# Input
FORMS += vpiano.ui connections.ui vpianoabout.ui preferences.ui
HEADERS += pianokey.h pianokeybd.h pianoscene.h vpiano.h \
           keyboardmap.h keylabel.h pianodefs.h \
           connections.h vpianoabout.h preferences.h
SOURCES += pianokey.cpp pianokeybd.cpp pianoscene.cpp vpiano.cpp \
           keylabel.cpp connections.cpp vpianoabout.cpp preferences.cpp vpianomain.cpp
RESOURCES += pianokeybd.qrc
