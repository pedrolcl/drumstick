/*
    Standard RIFF MIDI File dump program
    Copyright (C) 2006-2024, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFileInfo>
#include <QObject>
#include <QString>
#include <QTextCodec>
#include <QTextStream>
#include <cstdlib>
#include "dumprmi.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#define right Qt::right
#define left Qt::left
#define endl Qt::endl
#endif

using namespace drumstick::File;

int main(int argc, char **argv)
{
    const QString PGM_NAME = QStringLiteral("drumstick-dumprmi");
    const QString PGM_DESCRIPTION = QStringLiteral("Drumstick command line utility for decoding RMI (RIFF MIDI) files");
    QTextStream cerr(stderr, QIODevice::WriteOnly);

    DumpRmid spy;
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(PGM_NAME);
    QCoreApplication::setApplicationVersion(QStringLiteral(QT_STRINGIFY(VERSION)));

    QCommandLineParser parser;
    parser.setApplicationDescription(PGM_DESCRIPTION);
    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();
    QCommandLineOption extractOption({{"e", "extract"}, "Extracts the embedded standard MIDI file."});
    parser.addOption(extractOption);

    parser.addPositionalArgument("file", "Input RMI file name.", "files...");
    parser.process(app);

    if (parser.isSet(versionOption) || parser.isSet(helpOption)) {
        return 0;
    }

    spy.setExtract(parser.isSet(extractOption));

    QStringList fileNames, positionalArgs = parser.positionalArguments();
    if (positionalArgs.isEmpty()) {
        cerr << "Input file name(s) missing" << endl;
        parser.showHelp();
    }

    foreach(const QString& a, positionalArgs) {
        QFileInfo f(a);
        if (f.exists())
            fileNames += f.canonicalFilePath();
        else
            cerr << "File not found: " << a << endl;
    }

    int totalErrors = 0;
    foreach(const QString& file, fileNames) {
        spy.run(file);
        totalErrors += spy.numErrors();
    }
    return totalErrors;
}
