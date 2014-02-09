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

#include <QDebug>
#include <QUdpSocket>
#include "netmidioutput.h"

namespace drumstick {
namespace rt {

static QString DEFAULT_PUBLIC_NAME(QLatin1String("MIDI Out"));

class NetMIDIOutput::NetMIDIOutputPrivate
{
public:
    QUdpSocket *m_socket;
    quint16 m_port;
    QString m_publicName;
    QString m_currentOutput;
    QStringList m_outputDevices;
    QStringList m_excludedNames;

    NetMIDIOutputPrivate() :
        m_socket(0),
        m_port(0),
        m_publicName(DEFAULT_PUBLIC_NAME)
    {
        for(int i=MULTICAST_PORT; i<LAST_PORT; ++i) {
            m_outputDevices << QString::number(i);
        }
    }

    ~NetMIDIOutputPrivate()
    {
        close();
    }

    void open(QString portName)
    {
        int p = m_outputDevices.indexOf(portName);
        if (p > -1)
        {
            m_socket = new QUdpSocket();
            m_port = MULTICAST_PORT + p;
            m_currentOutput = portName;
            qDebug() << Q_FUNC_INFO << portName;
        }
    }

    void close()
    {
        delete m_socket;
        m_socket = 0;
        m_currentOutput.clear();
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
        if (m_socket == 0) {
            qDebug() << "udp socket is null";
            return;
        }
        m_socket->writeDatagram(message, MULTICAST_ADDRESS, m_port);
    }
};

NetMIDIOutput::NetMIDIOutput(QObject *parent) : MIDIOutput(parent),
  d(new NetMIDIOutputPrivate)
{ }

NetMIDIOutput::~NetMIDIOutput()
{
    delete d;
}

QString NetMIDIOutput::backendName()
{
    return QLatin1String("Network");
}

QString NetMIDIOutput::publicName()
{
    return d->m_publicName;
}

void NetMIDIOutput::setPublicName(QString name)
{
    d->m_publicName = name;
}

QStringList NetMIDIOutput::connections(bool advanced)
{
    Q_UNUSED(advanced)
    return d->m_outputDevices;
}

void NetMIDIOutput::setExcludedConnections(QStringList conns)
{
    Q_UNUSED(conns)
}

void NetMIDIOutput::open(QString name)
{
    d->open(name);
}

void NetMIDIOutput::close()
{
    d->close();
}

QString NetMIDIOutput::currentConnection()
{
    return d->m_currentOutput;
}

void NetMIDIOutput::sendNoteOff(int chan, int note, int vel)
{
    d->sendMessage(0x80 + chan, note, vel);
}

void NetMIDIOutput::sendNoteOn(int chan, int note, int vel)
{
    d->sendMessage(0x90 + chan, note, vel);
}

void NetMIDIOutput::sendKeyPressure(int chan, int note, int value)
{
    d->sendMessage(0xA0 + chan, note, value);
}

void NetMIDIOutput::sendController(int chan, int control, int value)
{
    d->sendMessage(0xB0 + chan, control, value);
}

void NetMIDIOutput::sendProgram(int chan, int program)
{
    d->sendMessage(0xC0 + chan, program);
}

void NetMIDIOutput::sendChannelPressure(int chan, int value)
{
    d->sendMessage(0xD0 + chan, value);
}

void NetMIDIOutput::sendPitchBend(int chan, int value)
{
    int lo = value % 0x80;
    int hi = value / 0x80;
    d->sendMessage(0xE0 + chan, lo, hi);
}

void NetMIDIOutput::sendSysex(const QByteArray &data)
{
    d->sendMessage(data);
}

void NetMIDIOutput::sendSystemMsg(const int status)
{
    d->sendMessage(status);
}

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    Q_EXPORT_PLUGIN2(drumstick_rt_net_out,NetMIDIOutput)
#endif

}}
