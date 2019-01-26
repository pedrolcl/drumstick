TEMPLATE = lib
TARGET = drumstick-alsa
DESTDIR = ../../build/lib
DEPENDPATH += . ../include
INCLUDEPATH += . ../include
include (../../global.pri)
QT -= gui
#QT += dbus
CONFIG += qt thread link_pkgconfig
static {
    CONFIG += staticlib create_prl
}
DEFINES += drumstick_alsa_EXPORTS #RTKIT_SUPPORT
QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_HIDESYMS
# Input
HEADERS += ../include/drumstick/alsaclient.h \
           ../include/drumstick/alsaevent.h \
           ../include/drumstick/alsaport.h \
           ../include/drumstick/alsaqueue.h \
           ../include/drumstick/alsatimer.h \
           ../include/drumstick/drumstick.h \
           ../include/drumstick/drumstickcommon.h \
           ../include/drumstick/macros.h \
           ../include/drumstick/playthread.h \
           ../include/drumstick/subscription.h
SOURCES += alsaclient.cpp \
           alsaevent.cpp \
           alsaport.cpp \
           alsaqueue.cpp \
           alsatimer.cpp \
           playthread.cpp \
           subscription.cpp

PKGCONFIG += alsa

