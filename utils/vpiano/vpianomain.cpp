/*
    Virtual Piano test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2020, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QDebug>
#include <QApplication>
#include <QFileInfo>
#include <QTextStream>
#include <QTranslator>
#include <QLibraryInfo>
#include <QCommandLineParser>
#include <drumstick/backendmanager.h>
#include <drumstick/settingsfactory.h>
#include "vpiano.h"

const QString PGM_DESCRIPTION = QObject::tr(
     "Drumstick Simple Virtual Piano\n"
     "Copyright (C) 2006-2020 Pedro López-Cabanillas\n"
     "This program comes with ABSOLUTELY NO WARRANTY;\n"
     "This is free software, and you are welcome to redistribute it\n"
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
Q_IMPORT_PLUGIN(SynthOutput)
#endif

#if defined(OSS_BACKEND)
Q_IMPORT_PLUGIN(OSSInput)
Q_IMPORT_PLUGIN(OSSOutput)
#endif

using namespace drumstick::rt;

int main(int argc, char *argv[])
{
    QTextStream cerr(stderr, QIODevice::WriteOnly);
    QCoreApplication::setOrganizationName("drumstick.sourceforge.net");
    QCoreApplication::setOrganizationDomain("drumstick.sourceforge.net");
    QCoreApplication::setApplicationName("VPiano");
    QCoreApplication::setApplicationVersion(QStringLiteral(QT_STRINGIFY(VERSION)));
    QCoreApplication::setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, false);
    QCoreApplication::setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, false);
    QApplication app(argc, argv);

    QLocale locale;
    QTranslator qtTranslator;
    qDebug() << "load Qt translator:" << locale.name() <<
             qtTranslator.load(locale, "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    QCoreApplication::installTranslator(&qtTranslator);

#if defined(Q_OS_WIN32)
    QString dataDir = QApplication::applicationDirPath() + "/";
#elif defined(Q_OS_MAC)
    QString dataDir = QApplication::applicationDirPath() + "/../Resources/";
#else
    QString dataDir = QApplication::applicationDirPath() + "/../share/drumstick/";
#endif

    QTranslator libTranslator;
    qDebug() << "load lib translator:" << locale.name() <<
             libTranslator.load(locale, "drumstick-widgets", "_", dataDir);
    QCoreApplication::installTranslator(&libTranslator);

    QTranslator appTranslator;
    qDebug() << "load app translator:" << locale.name() <<
             appTranslator.load(locale, "drumstick-vpiano", "_", dataDir);
    QCoreApplication::installTranslator(&appTranslator);

    QCommandLineParser parser;
    parser.setApplicationDescription(QString("%1 v.%2\n\n%3").arg(QCoreApplication::applicationName(),
        QCoreApplication::applicationVersion(), PGM_DESCRIPTION));
    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();
    QCommandLineOption portableOption({"p", "portable"}, QObject::tr("Portable settings mode"));
    parser.addOption(portableOption);
    parser.process(app);
    if (parser.isSet(versionOption) || parser.isSet(helpOption)) {
        return 0;
    }

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
        cerr << "Fatal error from a MIDI backend." << endl;
    }
    return 0;
}
