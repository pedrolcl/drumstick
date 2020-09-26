TEMPLATE = lib
TARGET = vpianoplugin
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target
DESTDIR = ../../build/lib/designer
DEPENDPATH += . ../include
INCLUDEPATH += . ../include
include (../../global.pri)
QT += widgets uiplugin
CONFIG += plugin
HEADERS += vpiano-plugin.h
SOURCES += vpiano-plugin.cpp
RESOURCES += vpiano-plugin.qrc
LIBS += -L$$OUT_PWD/../../build/lib/
LIBS += -l$$drumstickLib(drumstick-widgets)
