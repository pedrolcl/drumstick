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

#include "synthoutput.h"

namespace drumstick {
namespace rt {

const QString QSTR_FLUIDSYNTH(QLatin1String("FluidSynth"));

SynthOutput::SynthOutput(QObject *parent) : MIDIOutput(parent),
    m_synth(new SynthEngine(this))
{ }

QString SynthOutput::backendName()
{
    return QSTR_FLUIDSYNTH;
}

QString SynthOutput::publicName()
{
    return QSTR_FLUIDSYNTH;
}

void SynthOutput::setPublicName(QString name)
{
    Q_UNUSED(name)
}

QStringList SynthOutput::connections(bool advanced)
{
    Q_UNUSED(advanced)
    return QStringList(QSTR_FLUIDSYNTH);
}

void SynthOutput::setExcludedConnections(QStringList conns)
{
    Q_UNUSED(conns)
}

void SynthOutput::open(QString name)
{
    Q_UNUSED(name)
}

void SynthOutput::close()
{
}

QString SynthOutput::currentConnection()
{
    return QSTR_FLUIDSYNTH;
}

void SynthOutput::sendNoteOff(int chan, int note, int vel)
{
    m_synth->noteOff(chan, note, vel);
}

void SynthOutput::sendNoteOn(int chan, int note, int vel)
{
    m_synth->noteOn(chan, note, vel);
}

void SynthOutput::sendKeyPressure(int chan, int note, int value)
{
    Q_UNUSED(chan)
    Q_UNUSED(note)
    Q_UNUSED(value)
}

void SynthOutput::sendController(int chan, int control, int value)
{
    m_synth->controlChange(chan, control, value);
}

void SynthOutput::sendProgram(int chan, int program)
{
    m_synth->setInstrument(chan, program);
}

void SynthOutput::sendChannelPressure(int chan, int value)
{
    Q_UNUSED(chan)
    Q_UNUSED(value)
}

void SynthOutput::sendPitchBend(int chan, int value)
{
    m_synth->bender(chan, value);
}

void SynthOutput::sendSysex(const QByteArray &data)
{
    Q_UNUSED(data)
}

void SynthOutput::sendSystemMsg(const int status)
{
    Q_UNUSED(status)
}

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    Q_EXPORT_PLUGIN2(drumstick_rt_synth,SynthOutput)
#endif

}}
