TEMPLATE = app
TARGET = drumstick-guiplayer
QT += gui widgets #dbus
CONFIG += qt thread exceptions
static {
    CONFIG += link_prl
}
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
LIBS = -L../../build/lib -ldrumstick-file -ldrumstick-alsa -lasound
include (../../global.pri)
# Input
HEADERS += player.h guiplayer.h song.h playerabout.h
FORMS += guiplayer.ui playerabout.ui
SOURCES += playermain.cpp \
    player.cpp \
    guiplayer.cpp \
    song.cpp \
    playerabout.cpp
RESOURCES += guiplayer.qrc
