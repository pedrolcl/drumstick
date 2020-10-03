TEMPLATE = subdirs
SUBDIRS += fileTest \
           rtTest \
           widgetsTest

linux {
    SUBDIRS += \
        alsaTest1 \
        alsaTest2
}
