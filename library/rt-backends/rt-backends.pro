TEMPLATE = subdirs

dummy {
    SUBDIRS += dummy-in dummy-out
}

linux* {
    SUBDIRS += alsa-in alsa-out
    packagesExist(libpulse-simple) {
        SUBDIRS += eassynth
    }
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
else {
    packagesExist(fluidsynth) {
        SUBDIRS += synth
    }
}

win32 {
    SUBDIRS += win-in win-out
}

SUBDIRS += net-in net-out
