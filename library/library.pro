TEMPLATE = subdirs
SUBDIRS += \
    file \
    rt \
    rt-backends \
    vpiano-plugin \
    widgets

rt-backends.depends += rt
vpiano-plugin.depends += widgets

linux {
    SUBDIRS += alsa
    rt-backends.depends += alsa
}

macx {
    OTHER_FILES += Info.plist.lib
}
