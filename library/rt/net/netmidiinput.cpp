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

#include <QUdpSocket>
#include "netmidiinput.h"
#include "netmidiinput_p.h"
#include "netmidioutput.h"

namespace drumstick {
namespace rt {

/*static QString DEFAULT_PUBLIC_NAME(QLatin1String("MIDI In"));

class NetMIDIInput::NetMIDIInputPrivate : public QObject
{
    Q_OBJECT
public:
    NetMIDIInput *m_inp;
    NetMIDIOutput *m_out;
    QUdpSocket *m_socket;
    int m_thruEnabled;
    quint16 m_port;
    QString m_publicName;
    QString m_currentInput;
    QStringList m_inputDevices;
    QStringList m_excludedNames;

    NetMIDIInputPrivate(NetMIDIInput* inp) :
        m_inp(inp),
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

    void open(QString portName)
    {
        int p = m_inputDevices.indexOf(portName);
        if (p > -1)
        {
            m_socket = new QUdpSocket();
            m_port = MULTICAST_PORT + p;
            m_currentInput = portName;
            m_socket->bind(m_port, QUdpSocket::ShareAddress);
            m_socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);
            m_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
            m_socket->joinMulticastGroup(MULTICAST_ADDRESS);
            //QNetworkInterface iface;
            //if (iface.isValid()) {
            //  socket->setMulticastInterface(iface);
            //}
            connect(m_socket, SIGNAL(readyRead()), this, SLOT(processIncomingMessages()));
        }
    }

    void close()
    {
        delete m_socket;
        m_socket = 0;
        m_currentInput.clear();
    }

    void parse(const QByteArray& msg)
    {
        int pos = 0;
        while(pos < msg.length()) {
            int status = msg.at(pos);
            if (status < 0xF0) { // channel message
                int chan = status & 0x0F;
                status &= 0xF0;
                switch(status) {
                case 0x80: {
                    int m1 = msg.at(++pos);
                    int m2 = msg.at(++pos);
                    if(m_out != 0 && m_thruEnabled) {
                        m_out->sendNoteOff(chan, m1, m2);
                    }
                    emit m_inp->midiNoteOff(chan, m1, m2);
                    break; }
                case 0x90: {
                    int m1 = msg.at(++pos);
                    int m2 = msg.at(++pos);
                    if(m_out != 0 && m_thruEnabled) {
                        m_out->sendNoteOn(chan, m1, m2);
                    }
                    emit m_inp->midiNoteOn(chan, m1, m2);
                    break; }
                case 0xA0: {
                    int m1 = msg.at(++pos);
                    int m2 = msg.at(++pos);
                    if(m_out != 0 && m_thruEnabled) {
                        m_out->sendKeyPressure(chan, m1, m2);
                    }
                    emit m_inp->midiKeyPressure(chan, m1, m2);
                    break; }
                case 0xB0: {
                    int m1 = msg.at(++pos);
                    int m2 = msg.at(++pos);
                    if(m_out != 0 && m_thruEnabled) {
                        m_out->sendController(chan, m1, m2);
                    }
                    emit m_inp->midiController(chan, m1, m2);
                    break; }
                case 0xC0: {
                    int m1 = msg.at(++pos);
                    if(m_out != 0 && m_thruEnabled) {
                        m_out->sendProgram(chan, m1);
                    }
                    emit m_inp->midiProgram(chan, m1);
                    break; }
                case 0xD0: {
                    int m1 = msg.at(++pos);
                    if(m_out != 0 && m_thruEnabled) {
                        m_out->sendChannelPressure(chan, m1);
                    }
                    emit m_inp->midiChannelPressure(chan, m1);
                    break; }
                case 0xE0: {
                    int lo = msg.at(++pos);
                    int hi = msg.at(++pos);
                    if(m_out != 0 && m_thruEnabled) {
                        m_out->sendPitchBend(chan, lo + 127 * hi);
                    }
                    emit m_inp->midiPitchBend(chan, lo + 127 * hi);
                    break; }
                }
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

public slots:
    void processIncomingMessages()
    {
        while (m_socket->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(m_socket->pendingDatagramSize());
            m_socket->readDatagram(datagram.data(), datagram.size());
            parse(datagram);
        }
    }
};

*/

NetMIDIInput::NetMIDIInput(QObject *parent):
    MIDIInput(parent),
    d(new NetMIDIInputPrivate(this))
{ }

QString NetMIDIInput::backendName()
{
    return QLatin1String("Network");
}

QString NetMIDIInput::publicName()
{
    return d->m_publicName;
}

void NetMIDIInput::setPublicName(QString name)
{
    d->m_publicName = name;
}

QStringList NetMIDIInput::connections(bool advanced)
{
    Q_UNUSED(advanced)
    return d->m_inputDevices;
}

void NetMIDIInput::setExcludedConnections(QStringList conns)
{
    d->m_excludedNames = conns;
}

void NetMIDIInput::open(QString name)
{
    d->open(name);
}

void NetMIDIInput::close()
{
    d->close();
}

QString NetMIDIInput::currentConnection()
{
    return d->m_currentInput;
}

void NetMIDIInput::setMIDIThruDevice(MIDIOutput *device)
{
    d->m_out = device;
}

void NetMIDIInput::enableMIDIThru(bool enable)
{
    d->m_thruEnabled = enable;
}

bool NetMIDIInput::isEnabledMIDIThru()
{
    return d->m_thruEnabled && (d->m_out != 0);
}

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    Q_EXPORT_PLUGIN2(drumstick_rt_net_in,NetMIDIInput)
#endif

}}

