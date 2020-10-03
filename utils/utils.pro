TEMPLATE = subdirs
include (../global.pri)
DEFINES += VERSION=$$VERSION

SUBDIRS += \
   dumpsmf \
   dumpwrk \
   vpiano

linux {
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
