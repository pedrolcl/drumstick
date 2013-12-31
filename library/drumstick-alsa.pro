TEMPLATE = lib
TARGET = drumstick-alsa
DESTDIR = ../build/lib
OBJECTS_DIR = ../build/obj
MOC_DIR = ../build/moc
DEPENDPATH += . include src
INCLUDEPATH += . include
include (../global.pri)
QT += dbus
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += qt thread 
DEFINES += drumstick_alsa_EXPORTS RTKIT_SUPPORT
QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_HIDESYMS
# Input
HEADERS += include/alsaclient.h \
           include/alsaevent.h \
           include/alsaport.h \
           include/alsaqueue.h \
           include/alsatimer.h \
           include/drumstick.h \
           include/drumstickcommon.h \
           include/macros.h \
           include/playthread.h \
           include/subscription.h
SOURCES += src/alsaclient.cpp \
           src/alsaevent.cpp \
           src/alsaport.cpp \
           src/alsaqueue.cpp \
           src/alsatimer.cpp \
           src/playthread.cpp \
           src/subscription.cpp
