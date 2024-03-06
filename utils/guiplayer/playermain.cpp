/*
    SMF GUI Player test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2024, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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
#include <QDebug>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QTranslator>
#include <drumstick/sequencererror.h>

#include "guiplayer.h"

const char* PGM_DESCRIPTION = QT_TRANSLATE_NOOP("main", "ALSA Sequencer based MIDI file player");

const char* errorstr = QT_TRANSLATE_NOOP("main", "Fatal error from the ALSA sequencer. "
    "This usually happens when the kernel doesn't have ALSA support, "
    "or the device node (/dev/snd/seq) doesn't exists, "
    "or the kernel module (snd_seq) is not loaded. "
    "Please check your ALSA/MIDI configuration.");

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(GUIPlayer::QSTR_DOMAIN);
    QCoreApplication::setOrganizationDomain(GUIPlayer::QSTR_DOMAIN);
    QCoreApplication::setApplicationName(GUIPlayer::QSTR_APPNAME);
    QCoreApplication::setApplicationVersion(QStringLiteral(QT_STRINGIFY(VERSION)));
    QGuiApplication::setDesktopFileName("net.sourceforge.drumstick-guiplayer");

    QLocale locale;
    QTranslator qtTranslator;
    if ((locale.language() != QLocale::C) && (locale.language() != QLocale::English)) {
        if (qtTranslator.load(locale, "qt", "_",
            #if QT_VERSION < QT_VERSION_CHECK(6,0,0)
                              QLibraryInfo::location(QLibraryInfo::TranslationsPath)
            #else
                              QLibraryInfo::path(QLibraryInfo::TranslationsPath)
            #endif
                              )) {
            QCoreApplication::installTranslator(&qtTranslator);
        } else {
            qWarning() << "Unable to load Qt translator:" << locale.name();
        }
    }

#if defined(Q_OS_WIN32)
    QString dataDir = QApplication::applicationDirPath() + "/";
#elif defined(Q_OS_MAC)
    QString dataDir = QApplication::applicationDirPath() + "/../Resources/";
#else
    QString dataDir = QApplication::applicationDirPath() + "/../share/drumstick/";
#endif

    QTranslator appTranslator;
    if ((locale.language() != QLocale::C) && (locale.language() != QLocale::English)) {
        if (appTranslator.load(locale, "drumstick-guiplayer", "_", dataDir)) {
            QCoreApplication::installTranslator(&appTranslator);
        } else {
            qWarning() << "Unable to load app translator:" << locale.name();
        }
    }

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", PGM_DESCRIPTION));
    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();
    QCommandLineOption portOption({"p", "port"}, QCoreApplication::translate("main", "MIDI Out Connection."), "client:port");
    parser.addOption(portOption);
    parser.addPositionalArgument("file", QCoreApplication::translate("main", "Input SMF/KAR/RMI/WRK file name."), "file");
    parser.process(app);

    QStringList fileNames, positionalArgs = parser.positionalArguments();
    foreach(const QString& a, positionalArgs) {
        QFileInfo f(a);
        if (f.exists())
            fileNames += f.canonicalFilePath();
        else
            qWarning() << QCoreApplication::translate("main", "File not found: ") << a;
    }

    try {
        GUIPlayer w;
        if(parser.isSet(portOption)) {
            QString port = parser.value(portOption);
            w.subscribe(port);
        }
        if (!fileNames.isEmpty()) {
            w.openFile(fileNames.first());
        }
        w.show();
        return app.exec();
    } catch (const drumstick::ALSA::SequencerError& ex) {
        QMessageBox::critical(nullptr, QCoreApplication::translate("main", "Error"),
            QCoreApplication::translate("main", errorstr) + "\n" +
            QCoreApplication::translate("main", "Returned error was: ") + ex.qstrError() );
    } catch (...) {
        qWarning() << QCoreApplication::translate("main", errorstr);
    }
    return EXIT_FAILURE;
}
