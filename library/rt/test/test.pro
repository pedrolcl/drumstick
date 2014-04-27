TEMPLATE = app
TARGET = drumstick-rt-test
CONFIG += qt console
VERSION = 0.0.1
DESTDIR = ../../../build/bin
OBJECTS_DIR = ../../../build/obj
MOC_DIR = ../../../build/moc
DEPENDPATH += ../../include
INCLUDEPATH += ../../include
SOURCES += main.cpp

dummy {
    DEFINES += DUMMY_BACKEND
    LIBS += -L$$OUT_PWD/../../../build/lib -ldrumstick-rt-dummy
    PRE_TARGETDEPS += $$OUT_PWD/../../../build/lib/libdrumstick-rt-dummy.a
}

linux* {
    DEFINES += ALSA_BACKEND
    LIBS += -L$$OUT_PWD/../../../build/lib -ldrumstick-rt-alsa -ldrumstick-alsa -lasound
    PRE_TARGETDEPS += $$OUT_PWD/../../../build/lib/libdrumstick-rt-alsa.a
}

unix {
    DEFINES += OSS_BACKEND
    LIBS += -L$$OUT_PWD/../../../build/lib -ldrumstick-rt-oss
    PRE_TARGETDEPS += $$OUT_PWD/../../../build/lib/libdrumstick-rt-oss.a
}

macx {
    DEFINES += MAC_BACKEND
    LIBS += -L$$OUT_PWD/../../../build/lib -ldrumstick-rt-mac -framework CoreMidi -framework CoreFoundation
    PRE_TARGETDEPS += $$OUT_PWD/../../../build/lib/libdrumstick-rt-mac.a
}

win32 {
    DEFINES += WIN_BACKEND
    CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../../build/lib/release/ -ldrumstick-rt-win -lwinmm
    else:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../../build/lib/debug/ -ldrumstick-rt-win -lwinmm
    CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../../build/lib/release/drumstick-rt-win.lib
    else:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../../build/lib/debug/drumstick-rt-win.lib
}

DEFINES += NET_BACKEND
DEFINES += SYNTH_BACKEND
QT += network

win32 {
    CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../../build/lib/release/ -ldrumstick-rt-net -ldrumstick-rt-synth -lfluidsynth
    else:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../../build/lib/debug/ -ldrumstick-rt-net -ldrumstick-rt-synth -lfluidsynth
    CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../../build/lib/release/drumstick-rt-net.lib $$OUT_PWD/../../../build/lib/release/drumstick-rt-synth.lib
    else:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../../build/lib/debug/drumstick-rt-net.lib $$OUT_PWD/../../../build/lib/debug/drumstick-rt-synth.lib
}
else {
    LIBS += -L$$OUT_PWD/../../../build/lib -ldrumstick-rt-net -ldrumstick-rt-synth -lfluidsynth
    PRE_TARGETDEPS += $$OUT_PWD/../../../build/lib/libdrumstick-rt-net.a \
                      $$OUT_PWD/../../../build/lib/libdrumstick-rt-synth.a
}
