TEMPLATE = subdirs
SUBDIRS += fileTest1 \
           fileTest2 \
           fileTest3 \
           rtTest \
           widgetsTest

linux {
    SUBDIRS += \
        alsaTest1 \
        alsaTest2
}
