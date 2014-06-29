TEMPLATE = subdirs

dummy {
    SUBDIRS += dummy-in dummy-out
}

linux* {
    SUBDIRS += alsa-in alsa-out
}

unix:!macx {
    SUBDIRS += oss-in oss-out
}

macx {
    SUBDIRS += mac-in mac-out
}

win32 {
    SUBDIRS += win-in win-out
}

SUBDIRS += net-in net-out
SUBDIRS += synth
