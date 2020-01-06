TEMPLATE = lib
TARGET = $$qtLibraryTarget(drumstick-rt-fluidsynth)
DESTDIR = ../../../build/lib/drumstick
include (../../../global.pri)
CONFIG += c++11 plugin #create_prl
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
QT -= gui

HEADERS += synthengine.h \
           synthoutput.h

SOURCES += synthoutput.cpp synthengine.cpp

LIBS += -L$$OUT_PWD/../../../build/lib -ldrumstick-rt

macx {
    INCLUDEPATH += /Library/Frameworks/FluidSynth.framework/Headers
    QMAKE_LFLAGS += -F/Library/Frameworks
    LIBS += -framework FluidSynth
} else {
    CONFIG += link_pkgconfig
    packagesExist(fluidsynth) {
        PKGCONFIG += fluidsynth
    }
}

win32 {
    TARGET_EXT = .dll
}
