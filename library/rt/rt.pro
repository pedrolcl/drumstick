TEMPLATE = subdirs
CONFIG += ordered
dummy {
    SUBDIRS += dummy
}
linux* {
    SUBDIRS += alsa
}
#unix {
#    SUBDIRS += oss
#}
macx {
    SUBDIRS += mac
}
win32 {
    SUBDIRS += win
}
SUBDIRS += net
SUBDIRS += synth
SUBDIRS += test
