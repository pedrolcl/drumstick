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

#include <QCoreApplication>
#include <QFileInfo>
#include <QtPlugin>
#include <QPluginLoader>
#include <QString>
#include <QStringList>
#include <QDebug>

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

#if defined(NET_BACKEND)
Q_IMPORT_PLUGIN(NetMIDIInput)
Q_IMPORT_PLUGIN(NetMIDIOutput)
#endif

#if defined(DUMMY_BACKEND)
Q_IMPORT_PLUGIN(DummyInput)
Q_IMPORT_PLUGIN(DummyOutput)
#endif

using namespace drumstick::rt;

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
    qDebug() << "program" << exeInfo.fileName();

    QStringList cliFiles;
    foreach(const QString& a, app.arguments()) {
        if (!a.endsWith(exeInfo.fileName()))
            cliFiles << a;
    }
    qDebug() << "arguments" << cliFiles << endl;

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
        qDebug() << "Input Backend" << input->backendName();
        qDebug() << "public name" << input->publicName();
        qDebug() << "ports" << input->connections();
        qDebug() << "advanced ports" << input->connections(true) << endl;
    }

    foreach(MIDIOutput* output, outputsList) {
        qDebug() << "Output Backend" << output->backendName();
        qDebug() << "public name" << output->publicName();
        qDebug() << "ports" << output->connections();
        qDebug() << "advanced ports" << output->connections(true) << endl;
    }

    return 0;
}
