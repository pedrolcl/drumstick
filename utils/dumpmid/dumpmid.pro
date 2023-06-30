TEMPLATE = app
TARGET = drumstick-dumpmid
#QT += dbus
equals(QT_MAJOR_VERSION, 6) {
    QT += core5compat
}
CONFIG += c++11 cmdline
CONFIG += qt console thread exceptions
static {
    CONFIG += link_prl
}
DESTDIR = ../../build/bin
INCLUDEPATH += . ../../library/include
DEPENDPATH += . ../../library ../../library/include
include (../../global.pri)
# Input
HEADERS += dumpmid.h
SOURCES += dumpmid.cpp

LIBS = -L$$OUT_PWD/../../build/lib \
    -ldrumstick-alsa
