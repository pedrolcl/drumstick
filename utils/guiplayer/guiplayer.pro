TEMPLATE = app
TARGET = drumstick-guiplayer
QT += gui widgets dbus
CONFIG += qt thread exceptions link_prl
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include ../common 
LIBS = -L../../build/lib -ldrumstick-common -ldrumstick-file -ldrumstick-alsa -lasound
PRE_TARGETDEPS += $$OUT_PWD/../../build/lib/libdrumstick-common.a
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
