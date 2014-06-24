TEMPLATE = subdirs
CONFIG += ordered
include (../global.pri)
DEFINES += VERSION=$$VERSION

SUBDIRS += common \
           buildsmf \
           dumpove \
           dumpsmf \
           dumpwrk \
           test-rt \
           vpiano

linux* {
    SUBDIRS += \
           drumgrid \
           dumpmid \
           guiplayer \
           metronome \
           playsmf \
           sysinfo \
           testevents \
           timertest \
}
