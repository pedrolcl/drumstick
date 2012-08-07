TEMPLATE = subdirs
SUBDIRS += library
linux* {
    SUBDIRS += utils
}
