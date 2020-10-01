TEMPLATE = app
TARGET = drumstick-vpiano
QT += gui widgets network
CONFIG += qt c++11 thread exceptions
CONFIG += lrelease
DESTDIR = ../../build/bin
LRELEASE_DIR=.
include (../../global.pri)
INCLUDEPATH += . ../../library/include

# Input
FORMS += \
    vpiano.ui \
    connections.ui \
    vpianoabout.ui \
    preferences.ui

HEADERS += \
    vpiano.h \
    connections.h \
    vpianoabout.h \
    preferences.h \
    vpianosettings.h

SOURCES += \
    vpiano.cpp \
    connections.cpp \
    vpianoabout.cpp \
    preferences.cpp \
    vpianomain.cpp \
    vpianosettings.cpp

TRANSLATIONS += \
    drumstick-vpiano_en.ts \
    drumstick-vpiano_es.ts

# libs
macx:!static {
    QMAKE_LFLAGS += -F$$OUT_PWD/../../build/lib -L$$OUT_PWD/../../build/lib
    LIBS += -framework drumstick-rt -framework drumstick-widgets
    LIBS += -framework CoreFoundation
    ICON = ../../icons/drumstick.icns
    QMAKE_TARGET_BUNDLE_PREFIX = net.sourceforge
    QMAKE_BUNDLE = drumstick-vpiano
    QMAKE_INFO_PLIST = ../Info.plist.app
} else {
    LIBS += -L$$OUT_PWD/../../build/lib/
    LIBS += -l$$drumstickLib(drumstick-rt) -l$$drumstickLib(drumstick-widgets)
}

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

    linux* {
        DEFINES += LINUX_BACKEND
        LIBS += -ldrumstick-rt-alsa-in \
                -ldrumstick-rt-alsa-out \
                -ldrumstick-rt-eassynth \
                -ldrumstick-alsa \
                -lasound \
                -lsonivox
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
