/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2024 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include "ossinput.h"

namespace drumstick { namespace rt {

const QString OSSInput::DEFAULT_PUBLIC_NAME = QStringLiteral("MIDI In");

OSSInput::OSSInput(QObject *parent) : MIDIInput(parent),
    d(new OSSInputPrivate(this))
{ }

OSSInput::~OSSInput()
{
    delete d;
}

void OSSInput::initialize(QSettings *settings)
{
    Q_UNUSED(settings)
}

QString OSSInput::backendName()
{
    return "OSS";
}

QString OSSInput::publicName()
{
    return d->m_publicName;
}

void OSSInput::setPublicName(QString name)
{
    d->m_publicName = name;
}

QList<MIDIConnection> OSSInput::connections(bool advanced)
{
    d->reloadDeviceList(advanced);
    return d->m_inputDevices;
}

void OSSInput::setExcludedConnections(QStringList conns)
{
    Q_UNUSED(conns)
}

MIDIConnection OSSInput::currentConnection()
{
    return d->m_currentInput;
}

void OSSInput::open(const MIDIConnection& conn)
{
    d->open(conn);
}

void OSSInput::close()
{
    d->close();
}

void OSSInput::setMIDIThruDevice(MIDIOutput *device)
{
    d->setMIDIThruDevice(device);
    //d->m_out = device;
}

void OSSInput::enableMIDIThru(bool enable)
{
    d->m_thruEnabled = enable;
}

bool OSSInput::isEnabledMIDIThru()
{
    return d->m_thruEnabled && (d->m_out != nullptr);
}

} // namespace rt
} // namespace drumstick
