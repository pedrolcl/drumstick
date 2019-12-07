TEMPLATE = subdirs
SUBDIRS += \
    library \
    utils \
    tests
utils.depends = library
tests.depends = library
