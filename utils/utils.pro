TEMPLATE = subdirs
CONFIG += ordered
include (../global.pri)
DEFINES += VERSION=$$VERSION
QMAKE_LFLAGS += -F../build/lib
SUBDIRS += \
   dumpove \
   dumpsmf \
   dumpwrk \
   vpiano

linux* {
    SUBDIRS += \
       drumgrid \
       dumpmid \
       guiplayer \
       metronome \
       playsmf \
       sysinfo
}
macx {
    OTHER_FILES += Info.plist.app
}
