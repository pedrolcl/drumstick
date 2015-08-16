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

#include <QObject>
#include <QDebug>

#include "netmidiinput_p.h"
#include "netmidiinput.h"

namespace drumstick {
namespace rt {

static QString DEFAULT_PUBLIC_NAME(QLatin1String("MIDI In"));

NetMIDIInputPrivate::NetMIDIInputPrivate(QObject *parent) : QObject(parent),
    m_inp(qobject_cast<NetMIDIInput *>(parent)),
    m_out(0),
    m_socket(0),
    m_parser(0),
    m_thruEnabled(false),
    m_port(0),
    m_publicName(DEFAULT_PUBLIC_NAME),
    m_groupAddress(QHostAddress(STR_ADDRESS))
{
    for(int i=MULTICAST_PORT; i<LAST_PORT; ++i) {
        m_inputDevices << QString::number(i);
    }
}

void NetMIDIInputPrivate::open(QString portName)
{
    int p = m_inputDevices.indexOf(portName);
    if (p > -1)
    {
        m_socket = new QUdpSocket();
        m_parser = new MIDIParser(m_inp);
        m_port = MULTICAST_PORT + p;
        m_currentInput = portName;
        m_socket->bind(QHostAddress::AnyIPv4, m_port, QUdpSocket::ShareAddress);
        m_socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);
        m_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
        if (m_iface.isValid()) {
            m_socket->joinMulticastGroup(m_groupAddress, m_iface);
        } else {
            m_socket->joinMulticastGroup(m_groupAddress);
        }
        connect(m_socket, SIGNAL(readyRead()), this, SLOT(processIncomingMessages()));
        //qDebug() << Q_FUNC_INFO << portName;
    }
}

void NetMIDIInputPrivate::close()
{
    delete m_socket;
    delete m_parser;
    m_socket = 0;
    m_parser = 0;
    m_currentInput.clear();
}

void NetMIDIInputPrivate::initialize(QSettings *settings)
{
    if (settings != 0) {
        settings->beginGroup("Network");
        QString ifaceName = settings->value("interface", QString()).toString();
        QString address = settings->value("address", STR_ADDRESS).toString();
        settings->endGroup();
        if (!ifaceName.isEmpty()) {
            m_iface = QNetworkInterface::interfaceFromName(ifaceName);
        }
        if (!address.isEmpty()) {
            m_groupAddress.setAddress(address);
        }
    }
}

void NetMIDIInputPrivate::setMIDIThruDevice(MIDIOutput* device)
{
    m_out = device;
    if (m_parser != 0) {
        m_parser->setMIDIThruDevice(device);
    }
}

void NetMIDIInputPrivate::processIncomingMessages()
{
    while (m_socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_socket->pendingDatagramSize());
        m_socket->readDatagram(datagram.data(), datagram.size());
        if (m_parser != 0) {
            m_parser->parse(datagram);
        }
    }
}

}}
