/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2024 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QDir>
#include <QFile>

#include "ossoutput.h"

namespace drumstick { namespace rt {

const QString OSSOutput::DEFAULT_PUBLIC_NAME = QStringLiteral("MIDI Out");

class OSSOutput::OSSOutputPrivate
{
public:
    bool m_advanced;
    QIODevice *m_device;
    QString m_publicName;
    MIDIConnection m_currentOutput;
    QList<MIDIConnection> m_outputDevices;
    QStringList m_excludedNames;

    OSSOutputPrivate() :
        m_advanced(false),
        m_device(nullptr),
        m_publicName(DEFAULT_PUBLIC_NAME)
    {
        reloadDeviceList();
    }

    ~OSSOutputPrivate()
    {
        close();
    }

    void reloadDeviceList(bool advanced = false)
    {
        QDir dir("/dev");
        QStringList filters;
        m_advanced = advanced;
        filters << "dmmidi*" << "admmidi*";
        if (advanced) {
            filters << "midi*" << "amidi*";
        }
        dir.setNameFilters(filters);
        dir.setFilter(QDir::System);
        dir.setSorting(QDir::Name);
        m_outputDevices.clear();
        QFileInfoList listInfo = dir.entryInfoList();
        foreach(const QFileInfo &info, listInfo) {
            m_outputDevices << MIDIConnection(info.baseName(), info.absoluteFilePath());
        }
    }

    void open(const MIDIConnection& portName)
    {
        //qDebug() << Q_FUNC_INFO << portName;
        m_device = new QFile(portName.second.toString());
        m_device->open(QIODevice::WriteOnly | QIODevice::Unbuffered);
        m_currentOutput = portName;
    }

    void close()
    {
        if (m_device != nullptr) {
            m_device->close();
            delete m_device;
            m_device = nullptr;
        }
        m_currentOutput = MIDIConnection();
    }

    void sendMessage(int m0)
    {
        QByteArray m;
        m.resize(1);
        m[0] = m0;
        sendMessage(m);
    }

    void sendMessage(int m0, int m1)
    {
        QByteArray m;
        m.resize(2);
        m[0] = m0;
        m[1] = m1;
        sendMessage(m);
    }

    void sendMessage(int m0, int m1, int m2)
    {
        QByteArray m;
        m.resize(3);
        m[0] = m0;
        m[1] = m1;
        m[2] = m2;
        sendMessage(m);
    }

    void sendMessage(const QByteArray& message )
    {
        if (m_device == nullptr) {
            //qDebug() << "device is null";
            return;
        }
        m_device->write(message);
        //m_device->flush();
    }
};

OSSOutput::OSSOutput(QObject *parent) : MIDIOutput(parent),
    d(new OSSOutputPrivate)
{}

OSSOutput::~OSSOutput()
{
    delete d;
}

void OSSOutput::initialize(QSettings *settings)
{
    Q_UNUSED(settings)
}

QString OSSOutput::backendName()
{
    return QStringLiteral("OSS");
}

QString OSSOutput::publicName()
{
    return d->m_publicName;
}

void OSSOutput::setPublicName(QString name)
{
    d->m_publicName = name;
}

QList<MIDIConnection> OSSOutput::connections(bool advanced)
{
    d->reloadDeviceList(advanced);
    return d->m_outputDevices;
}

void OSSOutput::setExcludedConnections(QStringList conns)
{
    Q_UNUSED(conns)
}

void OSSOutput::open(const MIDIConnection& name)
{
    d->open(name);
}

void OSSOutput::close()
{
    d->close();
}

MIDIConnection OSSOutput::currentConnection()
{
    return d->m_currentOutput;
}

void OSSOutput::sendNoteOff(int chan, int note, int vel)
{
    d->sendMessage(MIDI_STATUS_NOTEOFF + chan, note, vel);}

void OSSOutput::sendNoteOn(int chan, int note, int vel)
{
    d->sendMessage(MIDI_STATUS_NOTEON + chan, note, vel);
}

void OSSOutput::sendKeyPressure(int chan, int note, int value)
{
    d->sendMessage(MIDI_STATUS_KEYPRESURE + chan, note, value);
}

void OSSOutput::sendController(int chan, int control, int value)
{
    d->sendMessage(MIDI_STATUS_CONTROLCHANGE + chan, control, value);
}

void OSSOutput::sendProgram(int chan, int program)
{
    d->sendMessage(MIDI_STATUS_PROGRAMCHANGE + chan, program);
}

void OSSOutput::sendChannelPressure(int chan, int value)
{
    d->sendMessage(MIDI_STATUS_CHANNELPRESSURE + chan, value);
}

void OSSOutput::sendPitchBend(int chan, int v)
{
    // -8192 <= v <= 8191; 0 <= value <= 16384
    int value = 8192 + v;
    d->sendMessage(MIDI_STATUS_PITCHBEND + chan, MIDI_LSB(value), MIDI_MSB(value));
}

void OSSOutput::sendSysex(const QByteArray &data)
{
    d->sendMessage(data);
}

void OSSOutput::sendSystemMsg(const int status)
{
    d->sendMessage(status);
}

} // namespace rt
} // namespace drumstick
