TEMPLATE = subdirs
CONFIG += ordered
linux* {
    SUBDIRS += alsaTest1
    SUBDIRS += alsaTest2
}
SUBDIRS += fileTest
SUBDIRS += rtTest
