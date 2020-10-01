/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2020 Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "dummyinput.h"

namespace drumstick {
namespace rt {

void DummyInput::initialize(QSettings* settings)
{
    Q_UNUSED(settings)
}

QString DummyInput::backendName()
{
    return QStringLiteral("DUMMY");
}

QString DummyInput::publicName()
{
    return QStringLiteral("DUMMY In");
}

void DummyInput::setPublicName(QString name)
{
    Q_UNUSED(name)
}

QList<MIDIConnection> DummyInput::connections(bool advanced)
{
    Q_UNUSED(advanced)
    return QList<MIDIConnection>();
}

void DummyInput::setExcludedConnections(QStringList conns)
{
    Q_UNUSED(conns)
}

MIDIConnection DummyInput::currentConnection()
{
    return MIDIConnection();
}

void DummyInput::open(const MIDIConnection& conn)
{
    Q_UNUSED(conn)
}

void DummyInput::close()
{
}

void DummyInput::setMIDIThruDevice(MIDIOutput *device)
{
    Q_UNUSED(device)
}

void DummyInput::enableMIDIThru(bool enable)
{
    Q_UNUSED(enable)
}

bool DummyInput::isEnabledMIDIThru()
{
    return false;
}


}}
