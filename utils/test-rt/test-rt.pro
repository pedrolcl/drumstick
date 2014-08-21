TEMPLATE = app
TARGET = drumstick-rt-test
CONFIG += qt console link_prl
DESTDIR = ../../build/bin
DEPENDPATH  += . ../../library/include
INCLUDEPATH += . ../../library/include ../common

SOURCES += main.cpp

macx {
    QMAKE_LFLAGS += -F$$OUT_PWD/../../build/lib -L$$OUT_PWD/../../build/lib
    LIBS += -framework drumstick-rt
    LIBS += -l$$qtLibraryTarget(drumstick-common)
}
else {
    LIBS += -L$$OUT_PWD/../../build/lib \
            -l$$qtLibraryTarget(drumstick-common) \
            -l$$qtLibraryTarget(drumstick-rt)
}

static {

LIBS += -L$$OUT_PWD/../../build/lib/drumstick

#dummy {
#    DEFINES += DUMMY_BACKEND
#    LIBS += -l$$qtLibraryTarget(drumstick-rt-dummy-in) \
#            -l$$qtLibraryTarget(drumstick-rt-dummy-out)
#}

linux* {
    DEFINES += ALSA_BACKEND
    LIBS += -l$$qtLibraryTarget(drumstick-rt-alsa-in) \
            -l$$qtLibraryTarget(drumstick-rt-alsa-out) \
            -l$$qtLibraryTarget(drumstick-alsa) \
            -lasound
}

unix:!macx {
    DEFINES += OSS_BACKEND
    LIBS += -l$$qtLibraryTarget(drumstick-rt-oss-in) \
            -l$$qtLibraryTarget(drumstick-rt-oss-out)
}

macx {
    DEFINES += MAC_BACKEND
    LIBS += -l$$qtLibraryTarget(drumstick-rt-mac-in) \
            -l$$qtLibraryTarget(drumstick-rt-mac-out) \
            -framework CoreMidi \
            -framework CoreFoundation
}

win32 {
    DEFINES += WIN_BACKEND
    LIBS += -l$$qtLibraryTarget(drumstick-rt-win-in) \
            -l$$qtLibraryTarget(drumstick-rt-win-out) \
            -lwinmm
}

DEFINES += NET_BACKEND
QT += network
LIBS += -l$$qtLibraryTarget(drumstick-rt-net-in) \
        -l$$qtLibraryTarget(drumstick-rt-net-out)

DEFINES += SYNTH_BACKEND
CONFIG += link_pkgconfig
LIBS += -l$$qtLibraryTarget(drumstick-rt-synth)
PKGCONFIG += fluidsynth
}
