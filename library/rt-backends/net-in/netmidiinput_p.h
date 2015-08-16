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

#ifndef NETMIDIINPUT_P_H
#define NETMIDIINPUT_P_H

#include <QObject>
#include <QUdpSocket>
#include <QNetworkInterface>
#include "midiparser.h"

namespace drumstick {
namespace rt {

class MIDIOutput;
class NetMIDIInput;

class NetMIDIInputPrivate : public QObject
{
    Q_OBJECT
public:
    NetMIDIInput *m_inp;
    MIDIOutput *m_out;
    QUdpSocket *m_socket;
    MIDIParser *m_parser;
    int m_thruEnabled;
    quint16 m_port;
    QString m_publicName;
    QHostAddress m_groupAddress;
    QString m_currentInput;
    QStringList m_inputDevices;
    QStringList m_excludedNames;
    QNetworkInterface m_iface;

    NetMIDIInputPrivate(QObject *parent = 0);

    void open(QString portName);
    void close();
    void initialize(QSettings* settings);
    void setMIDIThruDevice(MIDIOutput* device);

public slots:
    void processIncomingMessages();
};

}}
#endif // NETMIDIINPUT_P_H
