/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2022 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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
    MIDIConnection m_currentInput;
    QList<MIDIConnection> m_inputDevices;
    QStringList m_excludedNames;
    QNetworkInterface m_iface;
    bool m_ipv6;
    bool m_status;
    QStringList m_diagnostics;

    explicit NetMIDIInputPrivate(QObject *parent = nullptr);

    void open(const MIDIConnection& conn);
    void close();
    void initialize(QSettings* settings);
    void setMIDIThruDevice(MIDIOutput* device);

public slots:
    void processIncomingMessages();
};

}}
#endif // NETMIDIINPUT_P_H
