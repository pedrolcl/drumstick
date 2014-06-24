TEMPLATE = app
TARGET = drumstick-rt-test
CONFIG += qt console link_prl
DESTDIR = ../../../build/bin
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
SOURCES += main.cpp
LIBS += -L$$OUT_PWD/../../../build/lib

dummy {
    DEFINES += DUMMY_BACKEND
    LIBS += -l$$qtLibraryTarget(drumstick-rt-dummy)
}

linux* {
    DEFINES += ALSA_BACKEND
    LIBS += -l$$qtLibraryTarget(drumstick-rt-alsa) \
            -l$$qtLibraryTarget(drumstick-alsa) \
            -lasound
}

unix {
    DEFINES += OSS_BACKEND
    LIBS += -l$$qtLibraryTarget(drumstick-rt-oss)
}

macx {
    DEFINES += MAC_BACKEND
    LIBS += -l$$qtLibraryTarget(drumstick-rt-mac) \
            -framework CoreMidi \
            -framework CoreFoundation
}

win32 {
    DEFINES += WIN_BACKEND
    LIBS += -l$$qtLibraryTarget(drumstick-rt-win) \
            -lwinmm
}

DEFINES += NET_BACKEND
QT += network
LIBS += -l$$qtLibraryTarget(drumstick-rt-net)

#DEFINES += SYNTH_BACKEND
#LIBS += -l$$qtLibraryTarget(drumstick-rt-synth) \
#        -lfluidsynth
