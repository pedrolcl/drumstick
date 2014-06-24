DESTDIR = ../build/common
TEMPLATE = subdirs
CONFIG += ordered
include (../global.pri)
DEFINES += VERSION=$$VERSION
system($$QMAKE_MKDIR $$DESTDIR)
win32 {
    SUBDIRS += common \
           buildsmf \
           dumpove \
           dumpsmf \
           dumpwrk \
           vpiano
}
linux* {
    SUBDIRS += common \
           buildsmf \
           drumgrid \
           dumpmid \
           dumpove \
           dumpsmf \
           dumpwrk \
           guiplayer \
           metronome \
           playsmf \
           sysinfo \
           testevents \
           timertest \
           vpiano
}
macx {
    SUBDIRS += common \
           buildsmf \
           dumpove \
           dumpsmf \
           dumpwrk \
           vpiano
}
