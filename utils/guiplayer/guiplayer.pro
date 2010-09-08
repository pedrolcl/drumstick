TEMPLATE = app
TARGET = drumstick-guiplayer
QT += dbus
CONFIG += qt thread exceptions
DESTDIR = ../../build/bin
OBJECTS_DIR = ../../build/obj
MOC_DIR = ../../build/moc
RCC_DIR = ../../build/rcc
UI_DIR = ../../build/ui
INCLUDEPATH += . ../../library/include ../common ../../build/common
LIBS = -L../../build/lib -ldrumstick-common -ldrumstick-file -ldrumstick-alsa -lasound
# Input
HEADERS += player.h guiplayer.h song.h playerabout.h
FORMS += guiplayer.ui playerabout.ui
SOURCES += playermain.cpp \
    player.cpp \
    guiplayer.cpp \
    song.cpp \
    playerabout.cpp
RESOURCES += guiplayer.qrc
