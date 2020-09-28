TEMPLATE = app
TARGET = drumstick-guiplayer
QT += gui widgets #dbus
CONFIG += qt c++11 thread exceptions
CONFIG += lrelease
static {
    CONFIG += link_prl
}
DESTDIR = ../../build/bin
LRELEASE_DIR=.
INCLUDEPATH += . ../../library/include ../common 
LIBS = -L../../build/lib -ldrumstick-file -ldrumstick-alsa -lasound
include (../../global.pri)
# Input
HEADERS += player.h guiplayer.h song.h playerabout.h iconutils.h
FORMS += guiplayer.ui playerabout.ui
SOURCES += playermain.cpp \
    player.cpp \
    guiplayer.cpp \
    song.cpp \
    iconutils.cpp \
    playerabout.cpp
RESOURCES += guiplayer.qrc
TRANSLATIONS += \
    drumstick-guiplayer_en.ts \
    drumstick-guiplayer_es.ts
