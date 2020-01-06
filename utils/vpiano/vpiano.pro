TEMPLATE = app
TARGET = drumstick-vpiano
QT += gui widgets network
CONFIG += qt thread exceptions
DESTDIR = ../../build/bin
include (../../global.pri)
INCLUDEPATH += . ../../library/include ../common

# Input
FORMS += vpiano.ui connections.ui vpianoabout.ui preferences.ui fluidsettingsdialog.ui networksettingsdialog.ui \
    sonivoxsettingsdialog.ui \
    macsynthsettingsdialog.ui
HEADERS += pianokey.h pianokeybd.h pianoscene.h vpiano.h \
    keyboardmap.h keylabel.h pianodefs.h \
    connections.h vpianoabout.h preferences.h \
    fluidsettingsdialog.h networksettingsdialog.h \
    macsynthsettingsdialog.h \
    sonivoxsettingsdialog.h
SOURCES += pianokey.cpp pianokeybd.cpp pianoscene.cpp vpiano.cpp \
    keylabel.cpp connections.cpp vpianoabout.cpp preferences.cpp vpianomain.cpp \
    fluidsettingsdialog.cpp networksettingsdialog.cpp \
    macsynthsettingsdialog.cpp \
    sonivoxsettingsdialog.cpp
RESOURCES += pianokeybd.qrc

# libs
macx:!static {
    QMAKE_LFLAGS += -F$$OUT_PWD/../../build/lib -L$$OUT_PWD/../../build/lib
    LIBS += -framework drumstick-rt
    ICON = ../../icons/drumstick.icns
    QMAKE_TARGET_BUNDLE_PREFIX = net.sourceforge
    QMAKE_BUNDLE = drumstick-vpiano
    QMAKE_INFO_PLIST = ../Info.plist.app
} else {
    LIBS += -L$$OUT_PWD/../../build/lib/
    LIBS += -l$$drumstickLib(drumstick-rt)
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
