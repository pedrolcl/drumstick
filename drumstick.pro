TEMPLATE = subdirs
SUBDIRS += \
    library \
    utils \
    tests
utils.depends = library
tests.depends = library

requires(equals(QT_MAJOR_VERSION, 5)|equals(QT_MAJOR_VERSION, 6))

equals(QT_MAJOR_VERSION, 5):lessThan(QT_MINOR_VERSION, 9) {
    message("Cannot build VMPK with Qt $${QT_VERSION}")
    error("Use Qt 5.9 or newer")
}

equals(QT_MAJOR_VERSION, 6):lessThan(QT_MINOR_VERSION, 2) {
    message("Cannot build VMPK with Qt $${QT_VERSION}")
    error("Use Qt 6.2 or newer")
}
