/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2025 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QObject>

#include "netmidiinput.h"
#include "netmidiinput_p.h"

namespace drumstick { namespace rt {

NetMIDIInputPrivate::NetMIDIInputPrivate(QObject *parent) : QObject(parent),
    m_inp(qobject_cast<NetMIDIInput *>(parent)),
    m_out(nullptr),
    m_socket(nullptr),
    m_parser(nullptr),
    m_thruEnabled(false),
    m_port(0),
    m_publicName(NetMIDIInput::DEFAULT_PUBLIC_NAME),
    m_groupAddress(QHostAddress(NetMIDIInput::STR_ADDRESS_IPV4)),
    m_ipv6(false),
    m_status(false)
{
    for(int i=NetMIDIInput::MULTICAST_PORT; i<NetMIDIInput::LAST_PORT; ++i) {
        m_inputDevices << MIDIConnection(QString::number(i), i);
    }
}

void NetMIDIInputPrivate::open(const MIDIConnection& portName)
{
    int p = portName.second.toInt();
    if (p >= NetMIDIInput::MULTICAST_PORT && p < NetMIDIInput::LAST_PORT && m_status)
    {
        //qDebug() << Q_FUNC_INFO << portName;
        m_socket = new QUdpSocket();
        m_parser = new MIDIParser(m_inp);
        m_port = static_cast<quint16>(p);
        m_currentInput = portName;
        bool res = m_socket->bind(m_ipv6 ? QHostAddress::AnyIPv6 : QHostAddress::AnyIPv4, m_port, QUdpSocket::ShareAddress);
        if (res) {
#ifdef Q_OS_WIN
            // https://docs.microsoft.com/es-es/windows/desktop/WinSock/ip-multicast-2
            m_socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);
#endif
            if (m_iface.isValid()) {
                res = m_socket->joinMulticastGroup(m_groupAddress, m_iface);
            } else {
                res = m_socket->joinMulticastGroup(m_groupAddress);
            }
            connect(m_socket, &QUdpSocket::readyRead, this, &NetMIDIInputPrivate::processIncomingMessages);
            m_status = m_socket->isValid();
        } else {
            m_status = false;
            m_diagnostics << QString("Socket error. err: %1 = %2").arg(m_socket->error()).arg(m_socket->errorString());
        }
    }
}

void NetMIDIInputPrivate::close()
{
    delete m_socket;
    delete m_parser;
    m_socket = nullptr;
    m_parser = nullptr;
    m_currentInput = MIDIConnection();
    m_status = false;
    m_diagnostics.clear();
}

void NetMIDIInputPrivate::initialize(QSettings *settings)
{
    if (settings != nullptr) {
        m_status = false;
        m_diagnostics.clear();
        settings->beginGroup("Network");
        QString ifaceName = settings->value("interface", QString()).toString();
        m_ipv6 = settings->value("ipv6", false).toBool();
        QString address = settings->value("address", m_ipv6 ? NetMIDIInput::STR_ADDRESS_IPV6 : NetMIDIInput::STR_ADDRESS_IPV4).toString();
        settings->endGroup();
        if (!ifaceName.isEmpty()) {
            m_iface = QNetworkInterface::interfaceFromName(ifaceName);
        }
        if (address.isEmpty()) {
            m_groupAddress.setAddress(m_ipv6 ? NetMIDIInput::STR_ADDRESS_IPV6 : NetMIDIInput::STR_ADDRESS_IPV4);
        } else {
            m_groupAddress.setAddress(address);
        }
        m_status = m_groupAddress.isMulticast();
        if (!m_status) {
            m_diagnostics << QString("Invalid multicast address: %1").arg(address);
        }
    }
}

void NetMIDIInputPrivate::writeSettings(QSettings *settings)
{
    if (settings != nullptr) {
        settings->beginGroup("Network");
        settings->setValue("interface", m_iface.name());
        settings->setValue("ipv6", m_ipv6);
        settings->setValue("address", m_groupAddress.toString());
        settings->endGroup();
    }
}

void NetMIDIInputPrivate::setMIDIThruDevice(MIDIOutput* device)
{
    m_out = device;
    if (m_parser != nullptr) {
        m_parser->setMIDIThruDevice(device);
    }
}

void NetMIDIInputPrivate::processIncomingMessages()
{
    while (m_socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(static_cast<int>(m_socket->pendingDatagramSize()));
        m_socket->readDatagram(datagram.data(), datagram.size());
        if (m_parser != nullptr) {
            m_parser->parse(datagram);
        }
    }
}

} // namespace rt
} // namespace drumstick
