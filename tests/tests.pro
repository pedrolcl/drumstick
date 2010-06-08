DESTDIR = ../build/common
TEMPLATE = subdirs
SUBDIRS += buildsmf \
           dumpmid \
           dumpsmf \
           playsmf \
           smfplayer \
           sysinfo \
           testevents \
           timertest \
           vpiano \
           metronome \
           dumpwrk \
           drumgrid
VERSION=0.3.2svn
VERSIONH = $$sprintf(const QString PGM_VERSION(\"%1\");,$$VERSION)
system($$QMAKE_MKDIR $$DESTDIR)
win32 { 
    system(echo $$VERSIONH > ../build/common/cmdversion.h)
}
linux* { 
    system(echo \'$$VERSIONH\' > ../build/common/cmdversion.h)
}
