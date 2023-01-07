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

#include "netmidiinput.h"
#include "netmidiinput_p.h"
#include <QUdpSocket>

namespace drumstick { namespace rt {

const QString NetMIDIInput::DEFAULT_PUBLIC_NAME = QStringLiteral("MIDI In");
const QString NetMIDIInput::STR_ADDRESS_IPV4 = QStringLiteral("225.0.0.37");
const QString NetMIDIInput::STR_ADDRESS_IPV6 = QStringLiteral("ff12::37");
const int NetMIDIInput::MULTICAST_PORT = 21928;
const int NetMIDIInput::LAST_PORT = 21948;

NetMIDIInput::NetMIDIInput(QObject *parent):
    MIDIInput(parent),
    d(new NetMIDIInputPrivate(this))
{ }

void NetMIDIInput::initialize(QSettings *settings)
{
    d->initialize(settings);
}

QString NetMIDIInput::backendName()
{
    return QStringLiteral("Network");
}

QString NetMIDIInput::publicName()
{
    return d->m_publicName;
}

void NetMIDIInput::setPublicName(QString name)
{
    d->m_publicName = name;
}

QList<MIDIConnection> NetMIDIInput::connections(bool advanced)
{
    Q_UNUSED(advanced)
    return d->m_inputDevices;
}

void NetMIDIInput::setExcludedConnections(QStringList conns)
{
    d->m_excludedNames = conns;
}

void NetMIDIInput::open(const MIDIConnection& name)
{
    d->open(name);
}

void NetMIDIInput::close()
{
    d->close();
}

MIDIConnection NetMIDIInput::currentConnection()
{
    return d->m_currentInput;
}

void NetMIDIInput::setMIDIThruDevice(MIDIOutput *device)
{
    d->setMIDIThruDevice(device);
}

void NetMIDIInput::enableMIDIThru(bool enable)
{
    d->m_thruEnabled = enable;
}

bool NetMIDIInput::isEnabledMIDIThru()
{
    return d->m_thruEnabled && (d->m_out != nullptr);
}

QStringList NetMIDIInput::getDiagnostics()
{
    return d->m_diagnostics;
}

bool NetMIDIInput::getStatus()
{
    return d->m_status;
}

} // namespace rt
} // namespace drumstick

