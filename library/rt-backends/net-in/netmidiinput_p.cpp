/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2014 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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
#include "netmidioutput.h"

namespace drumstick {
namespace rt {

static QString DEFAULT_PUBLIC_NAME(QLatin1String("MIDI In"));

NetMIDIInputPrivate::NetMIDIInputPrivate(QObject *parent) : QObject(parent),
    m_inp(qobject_cast<NetMIDIInput *>(parent)),
    m_out(0),
    m_socket(0),
    m_thruEnabled(false),
    m_port(0),
    m_publicName(DEFAULT_PUBLIC_NAME)
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
        m_port = MULTICAST_PORT + p;
        m_currentInput = portName;
        m_socket->bind(QHostAddress::AnyIPv4, m_port, QUdpSocket::ShareAddress);
        m_socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);
        m_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
        m_socket->joinMulticastGroup(MULTICAST_ADDRESS);
        //QNetworkInterface iface;
        //if (iface.isValid()) {
        //  socket->setMulticastInterface(iface);
        //}
        connect(m_socket, SIGNAL(readyRead()), this, SLOT(processIncomingMessages()));
        qDebug() << Q_FUNC_INFO << portName;
    }
}

void NetMIDIInputPrivate::close()
{
    delete m_socket;
    m_socket = 0;
    m_currentInput.clear();
}

void NetMIDIInputPrivate::parse(const QByteArray& msg)
{
    qDebug() << Q_FUNC_INFO << msg.toHex();
    int pos = 0;
    while(pos < msg.length()) {
        uchar status = static_cast<unsigned>(msg.at(pos));
        if (status < 0xF0) { // channel message
            int chan = status & 0x0F;
            status &= 0xF0;
            switch(status) {
            case 0x80: {
                int m1 = static_cast<unsigned>(msg.at(++pos));
                int m2 = static_cast<unsigned>(msg.at(++pos));
                if(m_out != 0 && m_thruEnabled) {
                    m_out->sendNoteOff(chan, m1, m2);
                }
                emit m_inp->midiNoteOff(chan, m1, m2);
                break; }
            case 0x90: {
                int m1 = static_cast<unsigned>(msg.at(++pos));
                int m2 = static_cast<unsigned>(msg.at(++pos));
                if(m_out != 0 && m_thruEnabled) {
                    m_out->sendNoteOn(chan, m1, m2);
                }
                emit m_inp->midiNoteOn(chan, m1, m2);
                break; }
            case 0xA0: {
                int m1 = static_cast<unsigned>(msg.at(++pos));
                int m2 = static_cast<unsigned>(msg.at(++pos));
                if(m_out != 0 && m_thruEnabled) {
                    m_out->sendKeyPressure(chan, m1, m2);
                }
                emit m_inp->midiKeyPressure(chan, m1, m2);
                break; }
            case 0xB0: {
                int m1 = static_cast<unsigned>(msg.at(++pos));
                int m2 = static_cast<unsigned>(msg.at(++pos));
                if(m_out != 0 && m_thruEnabled) {
                    m_out->sendController(chan, m1, m2);
                }
                emit m_inp->midiController(chan, m1, m2);
                break; }
            case 0xC0: {
                int m1 = static_cast<unsigned>(msg.at(++pos));
                if(m_out != 0 && m_thruEnabled) {
                    m_out->sendProgram(chan, m1);
                }
                emit m_inp->midiProgram(chan, m1);
                break; }
            case 0xD0: {
                int m1 = static_cast<unsigned>(msg.at(++pos));
                if(m_out != 0 && m_thruEnabled) {
                    m_out->sendChannelPressure(chan, m1);
                }
                emit m_inp->midiChannelPressure(chan, m1);
                break; }
            case 0xE0: {
                int lo = static_cast<unsigned>(msg.at(++pos));
                int hi = static_cast<unsigned>(msg.at(++pos));
                int v = lo + hi * 0x80 - 0x2000;
                if(m_out != 0 && m_thruEnabled) {
                    m_out->sendPitchBend(chan, v);
                }
                emit m_inp->midiPitchBend(chan, v);
                break; }
            }
            pos++;
        } else { // system message
            if (status == 0xF0) {
                int p = msg.indexOf(0xF7, pos);
                int len = p < 0 ? p : p - pos + 1;
                if(m_out != 0 && m_thruEnabled) {
                    m_out->sendSysex(msg.mid(pos, len));
                }
                emit m_inp->midiSysex(msg.mid(pos, len));
                pos = p < 0 ? msg.length() : pos + len;
            } else {
                if(m_out != 0 && m_thruEnabled) {
                    m_out->sendSystemMsg(status);
                }
                if (status < 0xF7)
                    emit m_inp->midiSystemCommon(status);
                else if (status > 0xF7)
                    emit m_inp->midiSystemRealtime(status);
                pos++;
            }
        }
    }
}

void NetMIDIInputPrivate::processIncomingMessages()
{
    while (m_socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_socket->pendingDatagramSize());
        m_socket->readDatagram(datagram.data(), datagram.size());
        parse(datagram);
    }
}

}}
