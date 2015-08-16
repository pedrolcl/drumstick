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

#include <QUdpSocket>
#include "netmidiinput.h"
#include "netmidiinput_p.h"

namespace drumstick {
namespace rt {

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
    d->setMIDIThruDevice(device);
}

void NetMIDIInput::enableMIDIThru(bool enable)
{
    d->m_thruEnabled = enable;
}

bool NetMIDIInput::isEnabledMIDIThru()
{
    return d->m_thruEnabled && (d->m_out != 0);
}

}}

