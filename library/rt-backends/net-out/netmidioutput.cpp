/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2023 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include "netmidioutput.h"
#include <QNetworkInterface>
#include <QSettings>
#include <QUdpSocket>

namespace drumstick { namespace rt {

const QString NetMIDIOutput::DEFAULT_PUBLIC_NAME = QStringLiteral("MIDI Out");
const QString NetMIDIOutput::STR_ADDRESS_IPV4 = QStringLiteral("225.0.0.37");
const QString NetMIDIOutput::STR_ADDRESS_IPV6 = QStringLiteral("ff12::37");
const int NetMIDIOutput::MULTICAST_PORT = 21928;
const int NetMIDIOutput::LAST_PORT = 21948;

class NetMIDIOutput::NetMIDIOutputPrivate
{
public:
    QUdpSocket *m_socket;
    QString m_publicName;
    QHostAddress m_groupAddress;
    MIDIConnection m_currentOutput;
    QList<MIDIConnection> m_outputDevices;
    QStringList m_excludedNames;
    QNetworkInterface m_iface;
    quint16 m_port;
    bool m_ipv6;
    bool m_status;
    QStringList m_diagnostics;

    NetMIDIOutputPrivate() :
        m_socket(nullptr),
        m_publicName(DEFAULT_PUBLIC_NAME),
        m_groupAddress(QHostAddress(STR_ADDRESS_IPV4)),
        m_port(0),
        m_ipv6(false)
    {
        for(int i=MULTICAST_PORT; i<LAST_PORT; ++i) {
            m_outputDevices << MIDIConnection(QString::number(i), i);
        }
    }

    ~NetMIDIOutputPrivate()
    {
        close();
    }

    void initialize(QSettings* settings)
    {
        if (settings != nullptr) {
            m_status = false;
            m_diagnostics.clear();
            settings->beginGroup("Network");
            QString ifaceName = settings->value("interface", QString()).toString();
            m_ipv6 = settings->value("ipv6", false).toBool();
            QString address = settings->value("address", m_ipv6 ? STR_ADDRESS_IPV6 : STR_ADDRESS_IPV4).toString();
            settings->endGroup();
            if (!ifaceName.isEmpty()) {
                m_iface = QNetworkInterface::interfaceFromName(ifaceName);
            }
            if (address.isEmpty()) {
                m_groupAddress.setAddress(m_ipv6 ? STR_ADDRESS_IPV6 : STR_ADDRESS_IPV4);
            } else {
                m_groupAddress.setAddress(address);
            }
            m_status = m_groupAddress.isMulticast();
            if (!m_status) {
                m_diagnostics << QString("Invalid multicast address: %1").arg(address);
            }
        }
    }

    void writeSettings(QSettings *settings)
    {
        if (settings != nullptr) {
            settings->beginGroup("Network");
            settings->setValue("interface", m_iface.name());
            settings->setValue("ipv6", m_ipv6);
            settings->setValue("address", m_groupAddress.toString());
            settings->endGroup();
        }
    }

    void open(const MIDIConnection& portName)
    {
        //qDebug() << Q_FUNC_INFO << portName;
        int p = portName.second.toInt();
        if (p >= MULTICAST_PORT && p < LAST_PORT && m_status)
        {
            m_socket = new QUdpSocket();
            bool res = m_socket->bind(m_ipv6 ? QHostAddress::AnyIPv6 : QHostAddress::AnyIPv4, m_socket->localPort());
            if (res) {
                m_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
#ifdef Q_OS_UNIX
                m_socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);
#endif
                m_port = static_cast<quint16>(p);
                if (m_iface.isValid()) {
                    m_socket->setMulticastInterface(m_iface);
                }
                m_currentOutput = portName;
                m_status = m_socket->isValid();
            } else {
                m_status = false;
                m_diagnostics << QString("Socket error: %1 = %2").arg(m_socket->error()).arg(m_socket->errorString());
            }
        }
    }

    void close()
    {
        delete m_socket;
        m_socket = nullptr;
        m_currentOutput = MIDIConnection();
        m_status = false;
        m_diagnostics.clear();
    }

    void sendMessage(int m0)
    {
        QByteArray m;
        m.resize(1);
        m[0] = static_cast<char>(m0);
        sendMessage(m);
    }

    void sendMessage(int m0, int m1)
    {
        QByteArray m;
        m.resize(2);
        m[0] = static_cast<char>(m0);
        m[1] = static_cast<char>(m1);
        sendMessage(m);
    }

    void sendMessage(int m0, int m1, int m2)
    {
        QByteArray m;
        m.resize(3);
        m[0] = static_cast<char>(m0);
        m[1] = static_cast<char>(m1);
        m[2] = static_cast<char>(m2);
        sendMessage(m);
    }

    void sendMessage(const QByteArray& message )
    {
        //qDebug() << Q_FUNC_INFO << message.toHex() << m_groupAddress << m_port;
        if (m_socket == nullptr) {
            m_diagnostics << "udp socket is null";
            return;
        } else if (!m_socket->isValid() || m_socket->state() != QAbstractSocket::BoundState) {
            m_diagnostics << QString("udp socket has invalid state: %1 Error: %2 %3").arg(m_socket->state()).arg(m_socket->error()).arg(m_socket->errorString());
            return;
        }
        auto res = m_socket->writeDatagram(message, m_groupAddress, m_port);
        //qDebug() << Q_FUNC_INFO << "writeDatagram:" << res;
        if (res < 0) {
            m_diagnostics << QString("Error: %1 %2").arg(m_socket->error()).arg(m_socket->errorString());
        }
    }
};

NetMIDIOutput::NetMIDIOutput(QObject *parent) : MIDIOutput(parent),
  d(new NetMIDIOutputPrivate)
{ }

NetMIDIOutput::~NetMIDIOutput()
{
    delete d;
}

void NetMIDIOutput::initialize(QSettings *settings)
{
    d->initialize(settings);
}

QString NetMIDIOutput::backendName()
{
    return QStringLiteral("Network");
}

QString NetMIDIOutput::publicName()
{
    return d->m_publicName;
}

void NetMIDIOutput::setPublicName(QString name)
{
    d->m_publicName = name;
}

QList<MIDIConnection> NetMIDIOutput::connections(bool advanced)
{
    Q_UNUSED(advanced)
    return d->m_outputDevices;
}

void NetMIDIOutput::setExcludedConnections(QStringList conns)
{
    Q_UNUSED(conns)
}

void NetMIDIOutput::open(const MIDIConnection& name)
{
    d->open(name);
}

void NetMIDIOutput::close()
{
    d->close();
}

MIDIConnection NetMIDIOutput::currentConnection()
{
    return d->m_currentOutput;
}

void NetMIDIOutput::sendNoteOff(int chan, int note, int vel)
{
    d->sendMessage(MIDI_STATUS_NOTEOFF + chan, note, vel);
}

void NetMIDIOutput::sendNoteOn(int chan, int note, int vel)
{
    d->sendMessage(MIDI_STATUS_NOTEON + chan, note, vel);
}

void NetMIDIOutput::sendKeyPressure(int chan, int note, int value)
{
    d->sendMessage(MIDI_STATUS_KEYPRESURE + chan, note, value);
}

void NetMIDIOutput::sendController(int chan, int control, int value)
{
    d->sendMessage(MIDI_STATUS_CONTROLCHANGE + chan, control, value);
}

void NetMIDIOutput::sendProgram(int chan, int program)
{
    d->sendMessage(MIDI_STATUS_PROGRAMCHANGE + chan, program);
}

void NetMIDIOutput::sendChannelPressure(int chan, int value)
{
    d->sendMessage(MIDI_STATUS_CHANNELPRESSURE + chan, value);
}

void NetMIDIOutput::sendPitchBend(int chan, int v)
{
    // -8192 <= v <= 8191; 0 <= value <= 16384
    int value = 8192 + v;
    d->sendMessage(MIDI_STATUS_PITCHBEND + chan, MIDI_LSB(value), MIDI_MSB(value));
}

void NetMIDIOutput::sendSysex(const QByteArray &data)
{
    d->sendMessage(data);
}

void NetMIDIOutput::sendSystemMsg(const int status)
{
    d->sendMessage(status);
}

void NetMIDIOutput::writeSettings(QSettings *settings)
{
    d->writeSettings(settings);
}

QStringList NetMIDIOutput::getDiagnostics()
{
    return d->m_diagnostics;
}

bool NetMIDIOutput::getStatus()
{
    return d->m_status;
}

} // namespace rt
} // namespace drumstick

