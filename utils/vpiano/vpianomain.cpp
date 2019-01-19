/*
    Virtual Piano test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2019, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QApplication>
#include <QFileInfo>
#include <QSettings>
#include <QTextStream>
#include <QCommandLineParser>

#include "cmdversion.h"
#include "vpiano.h"
#include "backendmanager.h"

const QString PGM_DESCRIPTION("Drumstick Simple Virtual Piano");

int main(int argc, char *argv[])
{
    QTextStream cerr(stderr, QIODevice::WriteOnly);
    QCoreApplication::setOrganizationName("drumstick.sourceforge.net");
    QCoreApplication::setOrganizationDomain("drumstick.sourceforge.net");
    QCoreApplication::setApplicationName("VPiano");
    QCoreApplication::setApplicationVersion(PGM_VERSION);
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(PGM_DESCRIPTION);
    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();
    parser.process(app);
    if (parser.isSet(versionOption) || parser.isSet(helpOption)) {
        return 0;
    }

    QSettings settings;
    settings.beginGroup(QSTR_DRUMSTICKRT_GROUP);
    settings.setValue(QSTR_DRUMSTICKRT_PUBLICNAMEIN, QLatin1String("Virtual Piano IN"));
    settings.setValue(QSTR_DRUMSTICKRT_PUBLICNAMEOUT, QLatin1String("Virtual Piano OUT"));
    settings.endGroup();
    settings.sync();

    try {
        VPiano w;
        w.show();
        return app.exec();
    } catch (...) {
        cerr << "Fatal error from a MIDI backend." << endl;
    }
    return 0;
}
