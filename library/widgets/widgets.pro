TEMPLATE = lib
TARGET = drumstick-widgets
DESTDIR = ../../build/lib
DEPENDPATH += . ../include
INCLUDEPATH += . ../include
include (../../global.pri)
CONFIG += c++11 qt create_pc create_prl no_install_prl
static {
    CONFIG += staticlib
}
DEFINES += drumstick_widgets_EXPORTS
QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_HIDESYMS
QMAKE_PKGCONFIG_PREFIX = $$INSTALLBASE
QT += widgets network

FORMS += \
    fluidsettingsdialog.ui \
    networksettingsdialog.ui \
    sonivoxsettingsdialog.ui \
    macsynthsettingsdialog.ui

HEADERS += \
    ../include/drumstick/pianokeybd.h \
    ../include/drumstick/pianopalette.h \
    ../include/drumstick/configurationdialogs.h \
    pianokey.h \
    pianoscene.h \
    keyboardmap.h \
    keylabel.h \
    fluidsettingsdialog.h \
    networksettingsdialog.h \
    macsynthsettingsdialog.h \
    sonivoxsettingsdialog.h

SOURCES += \
    configurationdialogs.cpp \
    pianokey.cpp \
    pianokeybd.cpp \
    pianoscene.cpp \
    pianopalette.cpp \
    keylabel.cpp \
    fluidsettingsdialog.cpp \
    networksettingsdialog.cpp \
    macsynthsettingsdialog.cpp \
    sonivoxsettingsdialog.cpp

RESOURCES += pianokeybd.qrc
