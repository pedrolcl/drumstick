DESTDIR = ../build/common
TEMPLATE = subdirs
CONFIG += ordered
include (../global.pri)
DEFINES += VERSION=$$VERSION
#VERSIONH = $$sprintf(const QString PGM_VERSION(\"%1\");,$$VERSION)
system($$QMAKE_MKDIR $$DESTDIR)
win32 {
    #system(echo $$VERSIONH > ../build/common/cmdversion.h)
    SUBDIRS += common \
           buildsmf \
           dumpove \
           dumpsmf \
           dumpwrk \
           vpiano
}
linux* {
    #system(echo \'$$VERSIONH\' > ../build/common/cmdversion.h)
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
    #system(echo \'$$VERSIONH\' > ../build/common/cmdversion.h)
    SUBDIRS += common \
           buildsmf \
           dumpove \
           dumpsmf \
           dumpwrk \
           vpiano
}
