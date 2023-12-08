/*
    Sonivox EAS Synthesizer for Qt applications
    Copyright (C) 2016-2023, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QMutex>
#include "synthcontroller.h"
#include "synthrenderer.h"

namespace drumstick {
namespace rt {

SynthController::SynthController(QObject *parent) : MIDIOutput(parent)
{
    m_renderer = new SynthRenderer();
    m_renderer->moveToThread(&m_renderingThread);
    connect(&m_renderingThread, &QThread::started,  m_renderer, &SynthRenderer::run);
}

SynthController::~SynthController()
{
    //qDebug() << Q_FUNC_INFO;
    if (m_renderingThread.isRunning()) {
        stop();
    }
    delete m_renderer;
    m_renderer = nullptr;
}

void
SynthController::start()
{
    QMutex mutex;
    mutex.lock();
    m_renderer->setCondition(&m_rendering);
    m_renderingThread.start(QThread::HighPriority);
    m_rendering.wait(&mutex);
    mutex.unlock();
}

void
SynthController::stop()
{
    //qDebug() << Q_FUNC_INFO;
    m_renderer->stop();
    m_renderingThread.quit();
    m_renderingThread.wait();
}

void
SynthController::initialize(QSettings* settings)
{
    m_renderer->initialize(settings);
    //qDebug() << Q_FUNC_INFO;
}

QString SynthController::backendName()
{
    return SynthRenderer::QSTR_SONIVOXEAS;
}

QString SynthController::publicName()
{
    return SynthRenderer::QSTR_SONIVOXEAS;
}

void SynthController::setPublicName(QString name)
{
    Q_UNUSED(name)
}

QList<MIDIConnection> SynthController::connections(bool advanced)
{
    Q_UNUSED(advanced)
    return QList<MIDIConnection>{MIDIConnection(SynthRenderer::QSTR_SONIVOXEAS, SynthRenderer::QSTR_SONIVOXEAS)};
}

void SynthController::setExcludedConnections(QStringList conns)
{
    Q_UNUSED(conns)
}

void SynthController::open(const MIDIConnection& name)
{
    Q_UNUSED(name)
    //qDebug() << Q_FUNC_INFO << name;
    start();
}

void SynthController::close()
{
    //qDebug() << Q_FUNC_INFO;
    stop();
}

MIDIConnection SynthController::currentConnection()
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

void SynthController::sendPitchBend(int chan, int v)
{
    // -8192 <= v <= 8191; 0 <= value <= 16384
    int value = 8192 + v;
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

void SynthController::writeSettings(QSettings *settings)
{
    m_renderer->writeSettings(settings);
}

QStringList SynthController::getDiagnostics()
{
    return m_renderer->getDiagnostics();
}

bool SynthController::getStatus()
{
    return m_renderer->getStatus();
}

QString SynthController::getLibVersion()
{
    return m_renderer->getLibVersion();
}

QString SynthController::getSoundFont()
{
    return m_renderer->getSoundFont();
}

} // namespace rt
} // namespace drumstick
