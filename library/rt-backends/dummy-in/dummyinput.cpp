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

#include "dummyinput.h"

namespace drumstick {
namespace rt {

void DummyInput::initialize(QSettings* settings)
{
    Q_UNUSED(settings)
}

QString DummyInput::backendName()
{
    return "DUMMY";
}

QString DummyInput::publicName()
{
    return QString();
}

void DummyInput::setPublicName(QString name)
{
    Q_UNUSED(name)
}

QStringList DummyInput::connections(bool advanced)
{
    Q_UNUSED(advanced)
    return QStringList();
}

void DummyInput::setExcludedConnections(QStringList conns)
{
    Q_UNUSED(conns)
}

QString DummyInput::currentConnection()
{
    return QString();
}

void DummyInput::open(QString name)
{
    Q_UNUSED(name)
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
