/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2015 Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <cstdlib>
#include <QCoreApplication>
#include <QFileInfo>
#include <QtPlugin>
#include <QPluginLoader>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QDir>

#include "rtmidiinput.h"
#include "rtmidioutput.h"
#include "backendmanager.h"
#include "cmdlineargs.h"

using namespace drumstick::rt;

QTextStream cout(stdout, QIODevice::WriteOnly);

int main(int argc, char **argv)
{
    QCoreApplication::setOrganizationName("drumstick.sourceforge.net");
    QCoreApplication::setOrganizationDomain("drumstick.sourceforge.net");
    QCoreApplication::setApplicationName("drumstick-rt-test");
    QCoreApplication app(argc, argv);
    CmdLineArgs args;
    args.setStdQtArgs(true);
    args.parse(argc, argv);

    QFileInfo exeInfo(app.applicationFilePath());
    cout << "program=" << exeInfo.fileName();

    QSettings settings;
    settings.beginGroup(QSTR_DRUMSTICKRT_GROUP);
    settings.setValue(QSTR_DRUMSTICKRT_PUBLICNAMEIN, QLatin1String("TEST MIDI IN"));
    settings.setValue(QSTR_DRUMSTICKRT_PUBLICNAMEOUT, QLatin1String("TEST MIDI OUT"));
    settings.endGroup();
    settings.sync();

    QList<MIDIInput*> inputsList;
    QList<MIDIOutput*> outputsList;

    BackendManager man;
    man.refresh(&settings);
    inputsList = man.availableInputs();
    outputsList = man.availableOutputs();

    foreach(MIDIInput* input, inputsList) {
        cout << "Input Backend " << input->backendName() << endl;
        cout << "public name " << input->publicName() << endl;
        foreach(const QString& c, input->connections()) {
            cout << "port " << c << endl;
        }
        foreach(const QString& c, input->connections(true)) {
            cout << "advanced port " << c << endl;
        }
    }

    foreach(MIDIOutput* output, outputsList) {
        cout << "Output Backend " << output->backendName() << endl;
        cout << "public name " << output->publicName() << endl;
        foreach(const QString& c, output->connections()) {
            cout << "port " << c << endl;
        }
        foreach(const QString& c, output->connections(true)) {
            cout << "advanced port " << c << endl;
        }
    }

    cout.flush();
    return 0;
}
