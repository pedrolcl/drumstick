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

#include "fluidsynthoutput.h"

namespace drumstick { namespace rt {

FluidSynthOutput::FluidSynthOutput(QObject *parent) : MIDIOutput(parent)
{
    //qDebug() << Q_FUNC_INFO;
    m_synth = new FluidSynthEngine;
}

FluidSynthOutput::~FluidSynthOutput()
{
    //qDebug() << Q_FUNC_INFO;
    stop();
    delete m_synth;
}

void FluidSynthOutput::start()
{
    //qDebug() << Q_FUNC_INFO;
    m_synth->initialize();
}

void FluidSynthOutput::stop()
{
    //qDebug() << Q_FUNC_INFO;
    m_synth->stop();
}

QStringList FluidSynthOutput::getAudioDrivers()
{
    return m_synth->getAudioDrivers();
}

QStringList FluidSynthOutput::getDiagnostics()
{
    return m_synth->getDiagnostics();
}

QString FluidSynthOutput::getLibVersion()
{
    return m_synth->getLibVersion();
}

bool FluidSynthOutput::getStatus()
{
    return m_synth->getStatus();
}

QString FluidSynthOutput::getSoundFont()
{
    return m_synth->soundFont();
}

void FluidSynthOutput::initialize(QSettings *settings)
{
    //qDebug() << Q_FUNC_INFO;
    m_synth->readSettings(settings);
    stop();
    start();
}

QString FluidSynthOutput::backendName()
{
    return FluidSynthEngine::QSTR_FLUIDSYNTH;
}

QString FluidSynthOutput::publicName()
{
    return FluidSynthEngine::QSTR_FLUIDSYNTH;
}

void FluidSynthOutput::setPublicName(QString name)
{
    Q_UNUSED(name)
}

QList<MIDIConnection> FluidSynthOutput::connections(bool advanced)
{
    Q_UNUSED(advanced)
    return QList<MIDIConnection>{MIDIConnection(FluidSynthEngine::QSTR_FLUIDSYNTH, FluidSynthEngine::QSTR_FLUIDSYNTH)};
}

void FluidSynthOutput::setExcludedConnections(QStringList conns)
{
    Q_UNUSED(conns)
}

void FluidSynthOutput::open(const MIDIConnection& name)
{
    Q_UNUSED(name)
    //qDebug() << Q_FUNC_INFO;
    m_synth->open();
}

void FluidSynthOutput::close()
{
    //qDebug() << Q_FUNC_INFO;
    m_synth->close();
    stop();
}

MIDIConnection FluidSynthOutput::currentConnection()
{
    return m_synth->currentConnection();
}

void FluidSynthOutput::sendNoteOff(int chan, int note, int vel)
{
    m_synth->noteOff(chan, note, vel);
}

void FluidSynthOutput::sendNoteOn(int chan, int note, int vel)
{
    m_synth->noteOn(chan, note, vel);
}

void FluidSynthOutput::sendKeyPressure(int chan, int note, int value)
{
    m_synth->keyPressure(chan, note, value);
}

void FluidSynthOutput::sendController(int chan, int control, int value)
{
    m_synth->controlChange(chan, control, value);
}

void FluidSynthOutput::sendProgram(int chan, int program)
{
    m_synth->setInstrument(chan, program);
}

void FluidSynthOutput::sendChannelPressure(int chan, int value)
{
    m_synth->channelPressure(chan, value);
}

void FluidSynthOutput::sendPitchBend(int chan, int value)
{
    m_synth->bender(chan, value);
}

void FluidSynthOutput::sendSysex(const QByteArray &data)
{
    m_synth->sysex(data);
}

void FluidSynthOutput::sendSystemMsg(const int status)
{
    Q_UNUSED(status)
}

} // namespace rt
} // namespace drumstick
