TEMPLATE = lib
TARGET = $$qtLibraryTarget(drumstick-rt)
DESTDIR = ../../build/lib
DEPENDPATH += . ../include
INCLUDEPATH += . ../include
include (../../global.pri)
CONFIG += qt #create_prl
DEFINES += drumstick_rt_EXPORTS
QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_HIDESYMS
QT -= gui
# Input
HEADERS += \
    ../include/rtmidiinput.h \
    ../include/rtmidioutput.h

SOURCES += \
    backendmanager.cpp

win32 {
    TARGET_EXT = .dll
}

#LIBS += -L$$OUT_PWD/../../build/backends
#LIBS += -L$$OUT_PWD/../../build/lib

#dummy {
#    DEFINES += DUMMY_BACKEND
#    LIBS += -l$$qtLibraryTarget(drumstick-rt-dummy-in) \
#            -l$$qtLibraryTarget(drumstick-rt-dummy-out)
#}

#linux* {
#    DEFINES += ALSA_BACKEND
#    LIBS += -l$$qtLibraryTarget(drumstick-rt-alsa-in) \
#            -l$$qtLibraryTarget(drumstick-rt-alsa-out) \
#            -l$$qtLibraryTarget(drumstick-alsa) \
#            -lasound
#}

#unix {
#    DEFINES += OSS_BACKEND
#    LIBS += -l$$qtLibraryTarget(drumstick-rt-oss-in) \
#            -l$$qtLibraryTarget(drumstick-rt-oss-out)
#}

#macx {
#    DEFINES += MAC_BACKEND
#    LIBS += -l$$qtLibraryTarget(drumstick-rt-mac-in) \
#            -l$$qtLibraryTarget(drumstick-rt-mac-out) \
#            -framework CoreMidi \
#            -framework CoreFoundation
#}

#win32 {
#    DEFINES += WIN_BACKEND
#    LIBS += -l$$qtLibraryTarget(drumstick-rt-win-in) \
#            -l$$qtLibraryTarget(drumstick-rt-win-out) \
#            -lwinmm
#}

#DEFINES += NET_BACKEND
#QT += network
#LIBS += -l$$qtLibraryTarget(drumstick-rt-net-in) \
#        -l$$qtLibraryTarget(drumstick-rt-net-out)

#DEFINES += SYNTH_BACKEND
#LIBS += -l$$qtLibraryTarget(drumstick-rt-synth) \
#        -lfluidsynth
