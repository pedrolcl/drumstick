TEMPLATE = subdirs
SUBDIRS += drumstick-file.pro
linux* {
    SUBDIRS += drumstick-alsa.pro
}
