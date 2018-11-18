TEMPLATE = subdirs
CONFIG += ordered
linux* {
    SUBDIRS += alsa
}
SUBDIRS += file
SUBDIRS += rt-backends
SUBDIRS += rt

macx {
    OTHER_FILES += Info.plist.lib
}
