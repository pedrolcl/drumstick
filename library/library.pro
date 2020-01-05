TEMPLATE = subdirs
SUBDIRS += \
    file \
    rt-backends \
    rt

rt-backends.depends = rt

linux* {
    SUBDIRS += alsa
    rt-backends.depends = alsa
}

macx {
    OTHER_FILES += Info.plist.lib
}
