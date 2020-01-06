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

#include <QApplication>
#include <QFileInfo>
#include <QSettings>
#include <QTextStream>
#include <QCommandLineParser>
#include <drumstick/backendmanager.h>

#include "cmdversion.h"
#include "vpiano.h"

const QString PGM_DESCRIPTION("Drumstick Simple Virtual Piano");

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
