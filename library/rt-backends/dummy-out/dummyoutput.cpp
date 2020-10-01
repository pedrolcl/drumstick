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

#include "dummyoutput.h"

namespace drumstick {
namespace rt {

void DummyOutput::initialize(QSettings* settings)
{
    Q_UNUSED(settings)
}

QString DummyOutput::backendName()
{
    return QStringLiteral("DUMMY");
}

QString DummyOutput::publicName()
{
    return QStringLiteral("DUMMY Out");
}

void DummyOutput::setPublicName(QString name)
{
    Q_UNUSED(name)
}

QList<MIDIConnection> DummyOutput::connections(bool advanced)
{
    Q_UNUSED(advanced)
    return QList<MIDIConnection>();
}

void DummyOutput::setExcludedConnections(QStringList conns)
{
    Q_UNUSED(conns)
}

void DummyOutput::open(const MIDIConnection& conn)
{
    Q_UNUSED(conn)
}

void DummyOutput::close()
{
}

MIDIConnection DummyOutput::currentConnection()
{
    return MIDIConnection();
}

void DummyOutput::sendNoteOff(int chan, int note, int vel)
{
    Q_UNUSED(chan)
    Q_UNUSED(note)
    Q_UNUSED(vel)
}

void DummyOutput::sendNoteOn(int chan, int note, int vel)
{
    Q_UNUSED(chan)
    Q_UNUSED(note)
    Q_UNUSED(vel)
}

void DummyOutput::sendKeyPressure(int chan, int note, int value)
{
    Q_UNUSED(chan)
    Q_UNUSED(note)
    Q_UNUSED(value)
}

void DummyOutput::sendController(int chan, int control, int value)
{
    Q_UNUSED(chan)
    Q_UNUSED(control)
    Q_UNUSED(value)
}

void DummyOutput::sendProgram(int chan, int program)
{
    Q_UNUSED(chan)
    Q_UNUSED(program)
}

void DummyOutput::sendChannelPressure(int chan, int value)
{
    Q_UNUSED(chan)
    Q_UNUSED(value)
}

void DummyOutput::sendPitchBend(int chan, int value)
{
    Q_UNUSED(chan)
    Q_UNUSED(value)
}

void DummyOutput::sendSysex(const QByteArray &data)
{
    Q_UNUSED(data)
}

void DummyOutput::sendSystemMsg(const int status)
{
    Q_UNUSED(status)
}


}}
