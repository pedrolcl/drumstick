TEMPLATE = lib
TARGET = $$qtLibraryTarget(drumstick-alsa)
DESTDIR = ../../build/lib
DEPENDPATH += . ../include
INCLUDEPATH += . ../include
include (../../global.pri)
QT -= gui
QT += dbus
CONFIG += qt thread create_prl
DEFINES += drumstick_alsa_EXPORTS RTKIT_SUPPORT
QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_HIDESYMS
# Input
HEADERS += ../include/alsaclient.h \
           ../include/alsaevent.h \
           ../include/alsaport.h \
           ../include/alsaqueue.h \
           ../include/alsatimer.h \
           ../include/drumstick.h \
           ../include/drumstickcommon.h \
           ../include/macros.h \
           ../include/playthread.h \
           ../include/subscription.h
SOURCES += alsaclient.cpp \
           alsaevent.cpp \
           alsaport.cpp \
           alsaqueue.cpp \
           alsatimer.cpp \
           playthread.cpp \
           subscription.cpp

LIBS += -lasound
