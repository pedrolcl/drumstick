TEMPLATE = subdirs
SUBDIRS += \
    file \
    rt-backends \
    rt

linux* {
    SUBDIRS += alsa
    rt-backends.depends = alsa
}

macx {
    OTHER_FILES += Info.plist.lib
}
