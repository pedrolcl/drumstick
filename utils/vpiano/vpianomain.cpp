/*
    Virtual Piano test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2025, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QTranslator>
#include <QDebug>

#include "vpiano.h"

const char* PGM_DESCRIPTION = QT_TRANSLATE_NOOP("main",
     "Drumstick Simple Virtual Piano\n"
     "Copyright (C) 2006-2025 Pedro Lopez-Cabanillas\n"
     "This program comes with ABSOLUTELY NO WARRANTY; "
     "This is free software, and you are welcome to redistribute it "
     "under certain conditions; see the LICENSE file for details.");

#if defined(LINUX_BACKEND)
Q_IMPORT_PLUGIN(ALSAMIDIInput)
Q_IMPORT_PLUGIN(ALSAMIDIOutput)
Q_IMPORT_PLUGIN(SynthController)
#endif

#if defined(MAC_BACKEND)
Q_IMPORT_PLUGIN(MacMIDIInput)
Q_IMPORT_PLUGIN(MacMIDIOutput)
Q_IMPORT_PLUGIN(MacSynthOutput)
#endif

#if defined(WIN_BACKEND)
Q_IMPORT_PLUGIN(WinMIDIInput)
Q_IMPORT_PLUGIN(WinMIDIOutput)
#endif

#if defined(NET_BACKEND)
Q_IMPORT_PLUGIN(NetMIDIInput)
Q_IMPORT_PLUGIN(NetMIDIOutput)
#endif

#if defined(DUMMY_BACKEND)
Q_IMPORT_PLUGIN(DummyInput)
Q_IMPORT_PLUGIN(DummyOutput)
#endif

#if defined(FLUIDSYNTH_BACKEND)
Q_IMPORT_PLUGIN(FluidSynthOutput)
#endif

#if defined(OSS_BACKEND)
Q_IMPORT_PLUGIN(OSSInput)
Q_IMPORT_PLUGIN(OSSOutput)
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("drumstick.sourceforge.net");
    QCoreApplication::setOrganizationDomain("drumstick.sourceforge.net");
    QCoreApplication::setApplicationName("drumstick-vpiano");
    QCoreApplication::setApplicationVersion(QStringLiteral(QT_STRINGIFY(VERSION)));
    QCoreApplication::setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, false);
    QCoreApplication::setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, false);
    QGuiApplication::setDesktopFileName("net.sourceforge.drumstick-vpiano");

    QLocale locale;
    QTranslator qtTranslator;
    if ((locale.language() != QLocale::C) && (locale.language() != QLocale::English)) {
        QString qtTranslations =
            #if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
                QLibraryInfo::location(QLibraryInfo::TranslationsPath);
            #else
                QLibraryInfo::path(QLibraryInfo::TranslationsPath);
            #endif
        if (qtTranslator.load(locale, "qt", "_", qtTranslations)) {
            if (!QCoreApplication::installTranslator(&qtTranslator)) {
                qWarning() << "Failure installing qt translator" << locale;
            }
        } else {
            qWarning() << "Unable to load Qt translator:" << locale;
        }
    }

#if defined(Q_OS_WIN32)
    QString dataDir = QApplication::applicationDirPath() + "/";
#elif defined(Q_OS_MAC)
    QString dataDir = QApplication::applicationDirPath() + "/../Resources/";
#else
    QString dataDir = QApplication::applicationDirPath() + "/../share/drumstick/";
#endif

    QTranslator libTranslator, appTranslator;
    if ((locale.language() != QLocale::C) && (locale.language() != QLocale::English)) {
        if (libTranslator.load(locale, "drumstick-widgets", "_", dataDir)) {
            if (!QCoreApplication::installTranslator(&libTranslator)) {
                qWarning() << "installing lib translator" << locale << "failed";
            }
        } else {
            qWarning() << "Unable to load lib translator:" << locale << "from" << dataDir;
        }
        if (appTranslator.load(locale, "drumstick-vpiano", "_", dataDir)) {
            if (!QCoreApplication::installTranslator(&appTranslator)) {
                qWarning() << "installing app translator" << locale << "failed";
            }
        } else {
            qWarning() << "Unable to load app translator:" << locale << "from" << dataDir;
        }
    }

    QCommandLineParser parser;
    parser.setApplicationDescription(QString("%1 v.%2\n\n%3").arg(QCoreApplication::applicationName(),
        QCoreApplication::applicationVersion(), QCoreApplication::translate("main", PGM_DESCRIPTION)));
    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();
    QCommandLineOption portableOption({"p", "portable"}, QCoreApplication::translate("main", "Portable settings mode"));
    parser.addOption(portableOption);
    parser.process(app);

    try {
        VPiano w;
        if (parser.isSet(portableOption)) {
            w.setPortableConfig();
        } else {
            QSettings::setDefaultFormat(QSettings::NativeFormat);
        }
        w.show();
        return app.exec();
    } catch (...) {
        qWarning() << QCoreApplication::translate("main", "Fatal error from a MIDI backend.");
    }
    return EXIT_FAILURE;
}
