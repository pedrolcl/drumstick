TEMPLATE = subdirs
SUBDIRS += \
    fileTest \
    rtTest
linux* {
    SUBDIRS += \
        alsaTest1 \
        alsaTest2
}
