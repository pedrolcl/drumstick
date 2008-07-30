/*
    MIDI Sequencer C++ library 
    Copyright (C) 2006-2008, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include "commons.h"
#include "playthread.h"
#include "client.h"
#include "queue.h"
#include "event.h"

namespace ALSA 
{
namespace Sequencer 
{

const int TIMEOUT = 100;

SequencerOutputThread::SequencerOutputThread(MidiClient *seq, int portId) : QThread(),
    m_MidiClient(seq),
    m_PortId(portId),
    m_Stopped(false)
{
    m_Queue = m_MidiClient->getQueue();
    m_QueueId = m_Queue->getId();
    m_npfds = snd_seq_poll_descriptors_count(m_MidiClient->getHandle(), POLLOUT);
    m_pfds = (pollfd*) calloc(m_npfds, sizeof(pollfd));
    snd_seq_poll_descriptors(m_MidiClient->getHandle(), m_pfds, m_npfds, POLLOUT);
}

SequencerOutputThread::~SequencerOutputThread() 
{
    free(m_pfds);
}

bool 
SequencerOutputThread::stopped() 
{ 
    m_mutex.lock();
    bool bTmp = m_Stopped;
    m_mutex.unlock();
    return  bTmp;
}

void 
SequencerOutputThread::stop() 
{ 
    m_mutex.lock();
    m_Stopped = true;
    m_mutex.unlock();
}

void
SequencerOutputThread::sendEchoEvent(int tick)
{
    SystemEvent ev(SND_SEQ_EVENT_ECHO);
    ev.setSource(m_PortId);
    ev.setDestination(m_MidiClient->getClientId(), m_PortId);
    ev.scheduleTick(m_QueueId, tick, false);
    sendSongEvent(&ev);
}

void 
SequencerOutputThread::sendSongEvent(SequencerEvent* ev)
{
//    if (ev->getSequencerType() == SND_SEQ_EVENT_SYSEX) {
//        unsigned int j;
//        QString s;
//        SysExEvent* sev = dynamic_cast<SysExEvent*>(ev);
//        const char* data = sev->getData();
//        for (j = 0; j < sev->getLength(); ++j) {
//            s.append(QString("%1 ").arg((int)(data[j] & 0xff), 2, 16));
//        }
//        qDebug() << "sysex len:" << sev->getLength() << "data:" << s;
//    }
    
    while ((snd_seq_event_output(m_MidiClient->getHandle(), ev->getEvent()) < 0) 
            && !stopped()) 
        poll(m_pfds, m_npfds, TIMEOUT);
}

void
SequencerOutputThread::drainOutput()
{
    while ((snd_seq_drain_output(m_MidiClient->getHandle()) < 0) && !stopped())
        poll(m_pfds, m_npfds, TIMEOUT);
}

void
SequencerOutputThread::syncOutput()
{
    QueueStatus status = m_Queue->getStatus(); 
    while ((status.getEvents() > 0) && !stopped()) {
       usleep(TIMEOUT*1000);
       status = m_Queue->getStatus();
    }
}

void
SequencerOutputThread::shutupSound()
{
    int channel;
    for (channel = 0; channel < 16; ++channel) {
        ControllerEvent ev(channel, MIDI_CTL_ALL_SOUNDS_OFF, 0);
        ev.setSource(m_PortId);
        ev.setSubscribers();
        ev.setDirect();
        sendSongEvent(&ev);
    }
    drainOutput();
}

void SequencerOutputThread::run()
{
    unsigned int last_tick = 0;
    if (m_MidiClient != NULL) {
        try  {
            last_tick = getInitialPosition();
            if (last_tick == 0) {
                m_Queue->start();
            } else {
                m_Queue->setTickPosition(last_tick);
                m_Queue->continueRunning();
            }
            m_Stopped = false;
            while (!stopped() && hasNext()) {
                SequencerEvent* ev = nextEvent();
                if (getEchoResolution() > 0) {
                    while (last_tick < ev->getTick()) {
                        last_tick += getEchoResolution();
                        sendEchoEvent(last_tick);
                    }
                }
                sendSongEvent(ev);
            }
            if (stopped()) {
                m_Queue->clear();
                shutupSound();
            } else {
                drainOutput();
                syncOutput();
                if (stopped())
                    shutupSound();
                else
                    emit finished();
            }
            m_Queue->stop();
        } catch (...) {
            qWarning("exception in output thread");
        }
    }
}

}
}
