TEMPLATE = lib
TARGET = drumstick-vpiano-plugin
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

macx:!static {
    QMAKE_LFLAGS += -F$$OUT_PWD/../../build/lib -L$$OUT_PWD/../../build/lib
    LIBS += -framework drumstick-widgets
} else {
    LIBS += -L$$OUT_PWD/../../build/lib \
            -l$$drumstickLib(drumstick-widgets)
}
