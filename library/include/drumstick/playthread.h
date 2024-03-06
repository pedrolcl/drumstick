/*
    MIDI Sequencer C++ library
    Copyright (C) 2006-2024, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef DRUMSTICK_PLAYTHREAD_H
#define DRUMSTICK_PLAYTHREAD_H

#include "alsaevent.h"
#include <QThread>
#include <QReadWriteLock>

namespace drumstick { namespace ALSA {

/**
 * @file playthread.h
 * Sequencer output thread
 */

#if defined(DRUMSTICK_STATIC)
#define DRUMSTICK_ALSA_EXPORT
#else
#if defined(drumstick_alsa_EXPORTS)
#define DRUMSTICK_ALSA_EXPORT Q_DECL_EXPORT
#else
#define DRUMSTICK_ALSA_EXPORT Q_DECL_IMPORT
#endif
#endif

class MidiClient;
class MidiQueue;

/**
 * @addtogroup PlayThread ALSA Sequencer Output
 * @{
 *
 * @class SequencerOutputThread
 * Sequence player auxiliary class
 *
 * This class is used to implement an asynchronous sequence player using
 * ALSA sequencer scheduling
 *
 * Examples: guiplayer.cpp and playsmf.cpp
 */
class DRUMSTICK_ALSA_EXPORT SequencerOutputThread : public QThread
{
    Q_OBJECT

public:
    SequencerOutputThread(MidiClient *seq, int portId);
    virtual void run() override;
    /**
     * Gets the initial position in ticks of the sequence. The
     * default value zero means starting from the beginning.
     * @return Initial position (ticks)
     */
    virtual unsigned int getInitialPosition() { return 0; }
    /**
     * Gets the echo event resolution in ticks. This is the time
     * between echo events interleaved with the MIDI sequence. The default
     * value zero means that no echo events are sent at all.
     * @return Echo resolution (ticks)
     */
    virtual unsigned int getEchoResolution() { return 0; }
    /**
     * Check if there is one more event in the sequence.
     * This is a pure virtual method that must be overridden in the derived
     * class.
     * @return True if the sequence has another event.
     */
    virtual bool hasNext() = 0;
    /**
     * Gets the next event in the sequence.
     * This is a pure virtual function that must be overridden in the derived
     * class.
     * @return Pointer to the next SequencerEvent to be played.
     */
    virtual SequencerEvent* nextEvent() = 0;

    /**
     * Stops playing the current sequence.
     */
    virtual void stop();

Q_SIGNALS:
    /**
     * Signal emitted when the sequence play-back has finished.
     */
    void playbackFinished();

    /**
     * Signal emitted when the play-back has stopped.
     */
    void playbackStopped();

public Q_SLOTS:
    void start( QThread::Priority priority = InheritPriority );

protected:
    virtual void sendEchoEvent(int tick);
    virtual void sendSongEvent(SequencerEvent* ev);
    virtual void drainOutput();
    virtual void syncOutput();
    virtual bool stopRequested();

    MidiClient *m_MidiClient;   /**< MidiClient instance pointer */
    MidiQueue *m_Queue;         /**< MidiQueue instance pointer */
    int m_PortId;               /**< MidiPort numeric identifier */
    bool m_Stopped;             /**< Stopped status */
    int m_QueueId;              /**< MidiQueue numeric identifier */
    int m_npfds;                /**< Number of pollfd pointers */
    pollfd* m_pfds;             /**< Array of pollfd pointers */
    QReadWriteLock m_mutex;     /**< Mutex object used for synchronization */
};

/** @} */

}} /* namespace drumstick::ALSA */

#endif /*DRUMSTICK_PLAYTHREAD_H*/
