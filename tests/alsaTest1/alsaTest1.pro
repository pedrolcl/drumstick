QT       += testlib
QT       -= gui
TARGET = alsatest1
CONFIG   += c++11 cmdline
TEMPLATE = app
SOURCES += \
    alsatest1.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
INCLUDEPATH += . ../../library/include
LIBS = -L../../build/lib -ldrumstick-alsa -lasound
DESTDIR = ../../build/bin
