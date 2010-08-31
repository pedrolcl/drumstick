DESTDIR = ../build/common
TEMPLATE = subdirs
include (../global.pri)
VERSIONH = $$sprintf(const QString PGM_VERSION(\"%1\");,$$VERSION)
system($$QMAKE_MKDIR $$DESTDIR)
win32 { 
    system(echo $$VERSIONH > ../build/common/cmdversion.h)
    SUBDIRS += common \ 
           buildsmf \
           dumpove \
           dumpsmf \
           dumpwrk
}
linux* { 
    system(echo \'$$VERSIONH\' > ../build/common/cmdversion.h)
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
