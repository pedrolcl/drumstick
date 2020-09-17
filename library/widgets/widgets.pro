TEMPLATE = lib
TARGET = drumstick-widgets
DESTDIR = ../../build/lib
DEPENDPATH += . ../include
INCLUDEPATH += . ../include
include (../../global.pri)
CONFIG += c++11 qt create_pc create_prl no_install_prl lrelease
static {
    CONFIG += staticlib
}
LRELEASE_DIR=.
DEFINES += drumstick_widgets_EXPORTS
QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_HIDESYMS
QMAKE_PKGCONFIG_PREFIX = $$INSTALLBASE
QT += widgets network

FORMS += \
    fluidsettingsdialog.ui \
    networksettingsdialog.ui

HEADERS += \
    ../include/drumstick/pianokeybd.h \
    ../include/drumstick/pianopalette.h \
    ../include/drumstick/pianoscene.h \
    ../include/drumstick/configurationdialogs.h \
    ../include/drumstick/settingsfactory.h \
    pianokey.h \
    keylabel.h \
    fluidsettingsdialog.h \
    networksettingsdialog.h

SOURCES += \
    configurationdialogs.cpp \
    pianokey.cpp \
    pianokeybd.cpp \
    pianoscene.cpp \
    pianopalette.cpp \
    keylabel.cpp \
    fluidsettingsdialog.cpp \
    networksettingsdialog.cpp \
    settingsfactory.cpp

RESOURCES += pianokeybd.qrc

TRANSLATIONS += \
    translations/drumstick-widgets_en.ts \
    translations/drumstick-widgets_cs.ts \
    translations/drumstick-widgets_de.ts \
    translations/drumstick-widgets_es.ts \
    translations/drumstick-widgets_fr.ts \
    translations/drumstick-widgets_gl.ts \
    translations/drumstick-widgets_nl.ts \
    translations/drumstick-widgets_ru.ts \
    translations/drumstick-widgets_sr.ts \
    translations/drumstick-widgets_sv.ts \
    translations/drumstick-widgets_tr.ts \
    translations/drumstick-widgets_zh_CN.ts

macx {
    FORMS += macsynthsettingsdialog.ui
    HEADERS += macsynthsettingsdialog.h
    SOURCES += macsynthsettingsdialog.cpp
}

linux* {
    FORMS += sonivoxsettingsdialog.ui
    HEADERS += sonivoxsettingsdialog.h
    SOURCES += sonivoxsettingsdialog.cpp
}

macx:!static {
    TARGET = drumstick-widgets
    CONFIG += lib_bundle
    FRAMEWORK_HEADERS.version = Versions
    FRAMEWORK_HEADERS.files = $$HEADERS
    FRAMEWORK_HEADERS.path = Headers/drumstick
    QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
    #QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Frameworks/
    QMAKE_SONAME_PREFIX = @rpath
    QMAKE_TARGET_BUNDLE_PREFIX = net.sourceforge
    QMAKE_BUNDLE = drumstick-widgets
    QMAKE_INFO_PLIST = ../Info.plist.lib
}
