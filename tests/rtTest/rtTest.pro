TEMPLATE  = app
TARGET    = rtTest
QT       += testlib
QT       -= gui
CONFIG   += c++11 cmdline
include (../../global.pri)
SOURCES += rttest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
INCLUDEPATH += . ../../library/include/
DESTDIR = ../../build/bin

static {
    CONFIG += link_prl
    DEFINES += DRUMSTICK_STATIC

    DEFINES += NET_BACKEND
    LIBS += -L$$OUT_PWD/../../build/lib/drumstick/
    LIBS += -ldrumstick-rt-net-in \
            -ldrumstick-rt-net-out

    packagesExist(fluidsynth) {
        DEFINES += FLUIDSYNTH_BACKEND
        LIBS += -ldrumstick-rt-fluidsynth
        macx {
            QMAKE_LFLAGS += -F/Library/Frameworks
            LIBS += -framework FluidSynth
        } else {
            CONFIG += link_pkgconfig
            PKGCONFIG += fluidsynth
        }
    }

    linux {
        DEFINES += LINUX_BACKEND
        LIBS += -ldrumstick-rt-alsa-in \
                -ldrumstick-rt-alsa-out \
                -ldrumstick-rt-eassynth \
                -lsonivox \
                -ldrumstick-alsa \
                -lasound
    }

    unix:!macx {
        DEFINES += OSS_BACKEND
        LIBS += -ldrumstick-rt-oss-in \
                -ldrumstick-rt-oss-out
    }

    macx {
        DEFINES += MAC_BACKEND
        LIBS += -ldrumstick-rt-mac-in \
                -ldrumstick-rt-mac-out \
                -ldrumstick-rt-macsynth \
                -framework CoreMIDI \
                -framework CoreFoundation
    }

    win32 {
        DEFINES += WIN_BACKEND
        LIBS += -ldrumstick-rt-win-in \
                -ldrumstick-rt-win-out \
                -lwinmm
    }
}

macx:!static {
    QMAKE_LFLAGS += -F$$OUT_PWD/../../build/lib -L$$OUT_PWD/../../build/lib
    LIBS += -framework drumstick-rt
} else {
    LIBS += -L$$OUT_PWD/../../build/lib \
            -l$$drumstickLib(drumstick-rt)
}

