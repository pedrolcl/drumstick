TEMPLATE = subdirs

dummy {
    SUBDIRS += dummy-in dummy-out
}

linux* {
    SUBDIRS += alsa-in alsa-out eassynth
}

unix:!macx {
    SUBDIRS += oss-in oss-out
}

macx {
    SUBDIRS += mac-in mac-out macsynth
    exists(/Library/Frameworks/FluidSynth.framework/Headers/*) {
        SUBDIRS += synth
    }
}

win32 {
    SUBDIRS += win-in win-out
}

SUBDIRS += net-in net-out
#SUBDIRS += synth
