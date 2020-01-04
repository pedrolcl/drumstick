TEMPLATE = subdirs
SUBDIRS += fileTest
!static:SUBDIRS += rtTest

linux* {
    SUBDIRS += \
        alsaTest1 \
        alsaTest2
}
