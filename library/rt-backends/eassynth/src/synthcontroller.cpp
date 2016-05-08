/*
    Sonivox EAS Synthesizer for Qt applications
    Copyright (C) 2016, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <QDebug>
#include "synthcontroller.h"
#include "synthrenderer.h"

namespace drumstick {
namespace rt {

SynthController::SynthController(QObject *parent) : MIDIOutput(parent)
{
    m_renderer = new SynthRenderer();
    m_renderer->moveToThread(&m_renderingThread);
    connect(&m_renderingThread, &QThread::started,  m_renderer, &SynthRenderer::run);
    //connect(&m_renderingThread, &QThread::finished, m_renderer, &QObject::deleteLater);
}

SynthController::~SynthController()
{
    qDebug() << Q_FUNC_INFO;
    if (m_renderingThread.isRunning()) {
        stop();
    }
    delete m_renderer;
    m_renderer = 0;
}

void
SynthController::start()
{
    qDebug() << Q_FUNC_INFO;
    m_renderingThread.start(QThread::HighPriority);
}

void
SynthController::stop()
{
    qDebug() << Q_FUNC_INFO;
    m_renderer->stop();
    m_renderingThread.quit();
    m_renderingThread.wait();
}

void
SynthController::initialize(QSettings* settings)
{
    Q_UNUSED(settings)
}

QString SynthController::backendName()
{
    return QSTR_SONIVOXEAS;
}

QString SynthController::publicName()
{
    return QSTR_SONIVOXEAS;
}

void SynthController::setPublicName(QString name)
{
    Q_UNUSED(name)
}

QStringList SynthController::connections(bool advanced)
{
    Q_UNUSED(advanced)
    return QStringList(QSTR_SONIVOXEAS);
}

void SynthController::setExcludedConnections(QStringList conns)
{
    Q_UNUSED(conns)
}

void SynthController::open(QString name)
{
    Q_UNUSED(name)
    start();
}

void SynthController::close()
{
    stop();
}

QString SynthController::currentConnection()
{
    return m_renderer->connection();
}

void SynthController::sendNoteOff(int chan, int note, int vel)
{
    m_renderer->sendMessage(MIDI_STATUS_NOTEOFF + chan, note, vel);
}

void SynthController::sendNoteOn(int chan, int note, int vel)
{
    m_renderer->sendMessage(MIDI_STATUS_NOTEON + chan, note, vel);
}

void SynthController::sendKeyPressure(int chan, int note, int value)
{
    m_renderer->sendMessage(MIDI_STATUS_KEYPRESURE + chan, note, value);
}

void SynthController::sendController(int chan, int control, int value)
{
    m_renderer->sendMessage(MIDI_STATUS_CONTROLCHANGE + chan, control, value);
}

void SynthController::sendProgram(int chan, int program)
{
    m_renderer->sendMessage(MIDI_STATUS_PROGRAMCHANGE + chan, program);
}

void SynthController::sendChannelPressure(int chan, int value)
{
    m_renderer->sendMessage(MIDI_STATUS_CHANNELPRESSURE + chan, value);
}

void SynthController::sendPitchBend(int chan, int value)
{
    m_renderer->sendMessage(MIDI_STATUS_PITCHBEND + chan, MIDI_LSB(value), MIDI_MSB(value));
}

void SynthController::sendSysex(const QByteArray &data)
{
    Q_UNUSED(data)
}

void SynthController::sendSystemMsg(const int status)
{
    Q_UNUSED(status)
}

}}
