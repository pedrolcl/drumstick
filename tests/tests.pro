TEMPLATE = subdirs
SUBDIRS += fileTest
SUBDIRS += rtTest

linux* {
    SUBDIRS += \
        alsaTest1 \
        alsaTest2
}
