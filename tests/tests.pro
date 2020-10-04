TEMPLATE = subdirs
SUBDIRS += fileTest1 \
           fileTest2 \
           rtTest \
           widgetsTest

linux {
    SUBDIRS += \
        alsaTest1 \
        alsaTest2
}
