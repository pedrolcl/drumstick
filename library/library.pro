TEMPLATE = subdirs
SUBDIRS += \
    file \
    rt \
    rt-backends

rt-backends.depends += rt

linux* {
    SUBDIRS += alsa
    rt-backends.depends += alsa
}

macx {
    OTHER_FILES += Info.plist.lib
}
