TEMPLATE = lib
TARGET = $$qtLibraryTarget(drumstick-rt-synth)
DESTDIR = ../../../build/lib/drumstick
include (../../../global.pri)
CONFIG += plugin #create_prl static
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
QT -= gui

HEADERS += ../../include/rtmidiinput.h \
           ../../include/rtmidioutput.h \
           synthengine.h \
           synthoutput.h

SOURCES += synthoutput.cpp synthengine.cpp

macx {
    INCLUDEPATH += /Library/Frameworks/FluidSynth.framework/Headers
    QMAKE_LFLAGS += -F/Library/Frameworks
    LIBS += -framework FluidSynth
}
else {
    CONFIG += link_pkgconfig
    PKGCONFIG += fluidsynth
}

win32 {
    TARGET_EXT = .dll
}
