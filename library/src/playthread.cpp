/*
    MIDI Sequencer C++ library
    Copyright (C) 2006-2009, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include "playthread.h"
#include "alsaclient.h"
#include "alsaqueue.h"

/**
 * @file playthread.cpp
 * Implementation of a sequencer output thread
 */

namespace aseqmm {

/**
 * @addtogroup PlayThread
 * @{
 *
 * SequencerOutputThread provides MIDI sequence playback.
 *
 * This is an abstract class that must be extended providing
 * an implementation for the pure virtual methods:
 * SequencerOutputThread::hasNext() and SequencerOutputThread::nextEvent()
 * before using it for MIDI sequence playback. You can use any structure or
 * class you prefer to store the SequencerEvent objects, but they must be
 * provided ordered by time. A simplistic Song definition may be:
 *
 * @code
 * typedef QList<SequencerEvent> Song;
 * @endcode
 *
 * Using this class is optional. You may prefer another mechanism to
 * manage playback actions. This class uses a thread to manage the
 * sequence playback as a background task.
 * @}
 */

const int TIMEOUT = 100;

/**
 * Constructor
 * @param seq Existing MidiClient object pointer
 * @param portId Numeric input/output port identifier
 */
SequencerOutputThread::SequencerOutputThread(MidiClient *seq, int portId)
    : QThread(),
    m_MidiClient(seq),
    m_Queue(0),
    m_PortId(portId),
    m_Stopped(false),
    m_QueueId(0),
    m_npfds(0),
    m_pfds(0)
{
    if (m_MidiClient != NULL) {
        m_Queue = m_MidiClient->getQueue();
        m_QueueId = m_Queue->getId();
    }
}

/**
 * Checks if the playback task is stopRequested
 * @return True if the task is stopRequested
 */
bool
SequencerOutputThread::stopRequested()
{
    m_mutex.lockForRead();
    bool bTmp = m_Stopped;
    m_mutex.unlock();
    return  bTmp;
}

/**
 * Stops the playback task
 */
void
SequencerOutputThread::stop()
{
    m_mutex.lockForWrite();
    m_Stopped = true;
    m_mutex.unlock();
    while (isRunning())
        wait(TIMEOUT);
}

/**
 * Sends an echo event, with the same PortId as sender and destination.
 * @param tick Event schedule time in ticks.
 */
void
SequencerOutputThread::sendEchoEvent(int tick)
{
    if (!stopRequested() && m_MidiClient != NULL) {
        SystemEvent ev(SND_SEQ_EVENT_ECHO);
        ev.setSource(m_PortId);
        ev.setDestination(m_MidiClient->getClientId(), m_PortId);
        ev.scheduleTick(m_QueueId, tick, false);
        sendSongEvent(&ev);
    }
}

/**
 * Sends a SequencerEvent
 * @param ev SequencerEvent object pointer
 */
void
SequencerOutputThread::sendSongEvent(SequencerEvent* ev)
{
    if (m_MidiClient != NULL) {
        while (!stopRequested() &&
               (snd_seq_event_output_direct(m_MidiClient->getHandle(), ev->getHandle()) < 0))
            poll(m_pfds, m_npfds, TIMEOUT);
    }
}

/**
 * Flush the ALSA output buffer.
 */
void
SequencerOutputThread::drainOutput()
{
    if (m_MidiClient != NULL) {
        while (!stopRequested() &&
               (snd_seq_drain_output(m_MidiClient->getHandle()) < 0))
            poll(m_pfds, m_npfds, TIMEOUT);
    }
}

/**
 * Waits until the ALSA output queue is empty (all the events have been played.)
 */
void
SequencerOutputThread::syncOutput()
{
    if (!stopRequested() && m_MidiClient != NULL) {
        QueueStatus status = m_Queue->getStatus();
        while (!stopRequested() && (status.getEvents() > 0)) {
           usleep(TIMEOUT*1000);
           status = m_Queue->getStatus();
        }
    }
}

/**
 * Thread process loop
 */
void SequencerOutputThread::run()
{
    unsigned int last_tick;
    if (m_MidiClient != NULL) {
        try  {
            m_npfds = snd_seq_poll_descriptors_count(m_MidiClient->getHandle(), POLLOUT);
            m_pfds = (pollfd*) alloca(m_npfds * sizeof(pollfd));
            snd_seq_poll_descriptors(m_MidiClient->getHandle(), m_pfds, m_npfds, POLLOUT);
            last_tick = getInitialPosition();
            if (last_tick == 0) {
                m_Queue->start();
            } else {
                m_Queue->setTickPosition(last_tick);
                m_Queue->continueRunning();
            }
            while (!stopRequested() && hasNext()) {
                SequencerEvent* ev = nextEvent();
                if (getEchoResolution() > 0) {
                    while (!stopRequested() && (last_tick < ev->getTick())) {
                        last_tick += getEchoResolution();
                        sendEchoEvent(last_tick);
                    }
                }
                if (!stopRequested() && !SequencerEvent::isConnectionChange(ev))
                    sendSongEvent(ev);
            }
            if (stopRequested()) {
                m_Queue->clear();
                emit stopped();
            } else {
                drainOutput();
                syncOutput();
                if (stopRequested())
                    emit stopped();
                else
                    emit finished();
            }
            m_Queue->stop();
        } catch (...) {
            qWarning("exception in output thread");
        }
        m_npfds = 0;
        m_pfds = 0;
    }
}

void SequencerOutputThread::start( Priority priority )
{
    m_mutex.lockForWrite();
    m_Stopped = false;
    QThread::start( priority );
    m_mutex.unlock();
}

} /* namespace aseqmm */

