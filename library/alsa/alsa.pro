TEMPLATE = lib
TARGET = drumstick-alsa
DESTDIR = ../../build/lib
DEPENDPATH += . ../include
INCLUDEPATH += . ../include
include (../../global.pri)
QT -= gui
#QT += dbus
CONFIG += c++11 qt thread link_pkgconfig create_pc create_prl no_install_prl
static {
    CONFIG += staticlib
}
DEFINES += drumstick_alsa_EXPORTS #RTKIT_SUPPORT
QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_HIDESYMS
QMAKE_PKGCONFIG_PREFIX = $$INSTALLBASE
# Input
HEADERS += \
    ../include/drumstick.h \
    ../include/drumstick/alsaclient.h \
    ../include/drumstick/alsaevent.h \
    ../include/drumstick/alsaport.h \
    ../include/drumstick/alsaqueue.h \
    ../include/drumstick/alsatimer.h \
    ../include/drumstick/macros.h \
    ../include/drumstick/playthread.h \
    ../include/drumstick/subscription.h \
    ../include/drumstick/sequencererror.h \
    errorcheck.h

SOURCES += \
    alsaclient.cpp \
    alsaevent.cpp \
    alsaport.cpp \
    alsaqueue.cpp \
    alsatimer.cpp \
    playthread.cpp \
    sequencererror.cpp \
    subscription.cpp

linux:PKGCONFIG += alsa
