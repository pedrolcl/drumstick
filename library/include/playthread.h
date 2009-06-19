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

#ifndef INCLUDED_PLAYTHREAD_H
#define INCLUDED_PLAYTHREAD_H

#include "commons.h"
#include "event.h"
#include <QThread>
#include <QReadWriteLock>

namespace ALSA
{
namespace Sequencer
{

class MidiClient;
class MidiQueue;

class SequencerOutputThread : public QThread
{
    Q_OBJECT

public:
    SequencerOutputThread(MidiClient *seq, int portId);
    virtual void run();
    virtual unsigned int getInitialPosition() { return 0; }
    virtual unsigned int getEchoResolution() { return 0; }
    virtual bool hasNext() = 0; /* to be reimplemented */
    virtual SequencerEvent* nextEvent() = 0; /* to be reimplemented */
    bool stopped();
    void stop();

signals:
    void finished();

protected:
    void shutupSound();
    void sendEchoEvent(int tick);
    void sendSongEvent(SequencerEvent* ev);
    void drainOutput();
    void syncOutput();

    MidiClient *m_MidiClient;
    MidiQueue *m_Queue;
    int m_PortId;
    bool m_Stopped;
    int m_QueueId;
    int m_npfds;
    pollfd* m_pfds;
    QReadWriteLock m_mutex;
};

}
}

#endif /*INCLUDED_PLAYTHREAD_H*/
