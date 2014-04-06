/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2010 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include "rtmidiinput.h"
#include "rtmidioutput.h"

#if defined(ALSA_BACKEND)
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
namespace drumstick {
namespace rt {
Q_IMPORT_PLUGIN(drumstick_rt_alsa_in)
Q_IMPORT_PLUGIN(drumstick_rt_alsa_out)
}}
#else
Q_IMPORT_PLUGIN(ALSAMIDIInput)
Q_IMPORT_PLUGIN(ALSAMIDIOutput)
#endif
#endif

#if defined(MAC_BACKEND)
Q_IMPORT_PLUGIN(MacMIDIInput)
Q_IMPORT_PLUGIN(MacMIDIOutput)
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

using namespace drumstick::rt;

QTextStream cout(stdout, QIODevice::WriteOnly);

int main(int argc, char **argv)
{
    QCoreApplication::setOrganizationName("drumstick.sourceforge.net");
    QCoreApplication::setOrganizationDomain("drumstick.sourceforge.net");
    QCoreApplication::setApplicationName("drumstick-rt-test");
    QCoreApplication app(argc, argv);

    QString name_in(QLatin1String("TEST MIDI IN"));
    QString name_out(QLatin1String("TEST MIDI OUT"));
    QStringList names;
    names << name_in;
    names << name_out;

    QList<MIDIInput*> inputsList;
    QList<MIDIOutput*> outputsList;

    QFileInfo exeInfo(app.applicationFilePath());
    cout << "program " << exeInfo.fileName() << endl;

    QStringList cliFiles;
    foreach(const QString& a, app.arguments()) {
        if (!a.endsWith(exeInfo.fileName()))
            cliFiles << a;
    }
    foreach(const QString& c, cliFiles) {
        cout << "argument " << c << endl;
    }

    foreach(QObject* obj, QPluginLoader::staticInstances()) {
        if (obj != 0) {
            MIDIInput *input = qobject_cast<MIDIInput*>(obj);
            if (input != 0) {
                input->setPublicName(name_in);
                input->setExcludedConnections(names);
                inputsList << input;
            } else {
                MIDIOutput *output = qobject_cast<MIDIOutput*>(obj);
                if (output != 0) {
                    output->setPublicName(name_out);
                    output->setExcludedConnections(names);
                    outputsList << output;
                }
            }
        }
    }

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
