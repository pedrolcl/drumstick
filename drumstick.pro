TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS += library
linux* {
    SUBDIRS += utils
}
