DESTDIR = ../build/common
TEMPLATE = subdirs
include (../global.pri)
VERSIONH = $$sprintf(const QString PGM_VERSION(\"%1\");,$$VERSION)
system($$QMAKE_MKDIR $$DESTDIR)
win32 { 
    system(echo $$VERSIONH > ../build/common/cmdversion.h)
    SUBDIRS += common \ 
           buildsmf \
           dumpsmf \
           dumpwrk
}
linux* { 
    system(echo \'$$VERSIONH\' > ../build/common/cmdversion.h)
    SUBDIRS += common \ 
           buildsmf \
           dumpmid \
           dumpsmf \
           playsmf \
           guiplayer \
           sysinfo \
           testevents \
           timertest \
           vpiano \
           metronome \
           dumpwrk \
           drumgrid
}
