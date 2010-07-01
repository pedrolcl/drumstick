/*
    MIDI Sequencer C++ library 
    Copyright (C) 2006-2010, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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
 
#ifndef DRUMSTICK_ALSAQUEUE_H
#define DRUMSTICK_ALSAQUEUE_H

#include "drumstickcommon.h"
/**
 * @file alsaqueue.h
 * Classes managing ALSA Sequencer queues
 * @defgroup ALSAQueue ALSA Sequencer Queues
 * @{
 */

namespace drumstick {

/**
 * This is the value for the base skew used in ALSA. It is not possible
 * to assign an arbitrary value (ALSA version <= 1.0.20).
 */
#define SKEW_BASE 0x10000

class MidiClient;
class TimerId;

/**
 * Queue information container.
 *
 * This class is used to hold some properties about an ALSA queue object.
 */
class DRUMSTICK_EXPORT QueueInfo
{
    friend class MidiQueue;

public:
    QueueInfo();
    QueueInfo(const QueueInfo& other);
    QueueInfo(snd_seq_queue_info_t* other);
    virtual ~QueueInfo();
    QueueInfo* clone();
    QueueInfo& operator=(const QueueInfo& other);
    int getInfoSize() const;
    
    int getId();
    QString getName();
    int getOwner();
    bool isLocked();
    unsigned int getFlags();
    
    void setName(QString value);
    void setOwner(int value);
    void setLocked(bool locked);
    void setFlags(unsigned int value);

private:
    snd_seq_queue_info_t* m_Info;
};

/**
 * Queue status container.
 *
 * This class is used to retrieve some status information from an ALSA queue.
 */
class DRUMSTICK_EXPORT QueueStatus
{
    friend class MidiQueue;

public:
    QueueStatus();
    QueueStatus(const QueueStatus& other);
    QueueStatus(snd_seq_queue_status_t* other);
    virtual ~QueueStatus();
    QueueStatus* clone();
    QueueStatus& operator=(const QueueStatus& other);
    int getInfoSize() const;

    int getId();
    int getEvents();
    const snd_seq_real_time_t* getRealtime();
    unsigned int getStatusBits();
    bool isRunning();
    double getClockTime();
    snd_seq_tick_time_t getTickTime();

private:
    snd_seq_queue_status_t* m_Info;
};

/**
 * Queue tempo container.
 *
 * This class is used to hold some tempo properties of an ALSA queue object.
 * The queue's resolution defines the meaning of the musical time, in ticks. It
 * is expressed in PPQ (parts per quarter), or ticks in a quarter note (crotchet).
 * The nominal tempo is usually expressed in BPM (beats per minute), or Maelzel
 * metronome units. It can be also given in microseconds per beat. The tempo skew
 * factor is given as two integer numbers: skew value and skew base, being the
 * factor the quotient of both quantities = value / base. Currently (ALSA <= 1.0.20)
 * you can only use the base constant 0x10000 (decimal 65536).
 */
class DRUMSTICK_EXPORT QueueTempo
{
    friend class MidiQueue;

public:
    QueueTempo();
    QueueTempo(const QueueTempo& other);
    QueueTempo(snd_seq_queue_tempo_t* other);
    virtual ~QueueTempo(); 
    QueueTempo* clone();
    QueueTempo& operator=(const QueueTempo& other);
    int getInfoSize() const;

    int getId();
    int getPPQ();
    unsigned int getSkewValue();
    unsigned int getSkewBase();
    unsigned int getTempo();
    void setPPQ(int value);
    void setSkewValue(unsigned int value);
    void setTempo(unsigned int value);
    
    float getNominalBPM();
    float getRealBPM();
    void setTempoFactor(float value);
    void setNominalBPM(float value);

protected:
    void setSkewBase(unsigned int value);
    
private:
    snd_seq_queue_tempo_t* m_Info;
};

/**
 * Queue timer container.
 *
 * This class is used to hold some properties about the Timer used with an ALSA
 * queue object.
 */
class DRUMSTICK_EXPORT QueueTimer
{
    friend class MidiQueue;

public:
    QueueTimer();
    QueueTimer(const QueueTimer& other);
    QueueTimer(snd_seq_queue_timer_t* other);
    virtual ~QueueTimer(); 
    QueueTimer* clone();
    QueueTimer& operator=(const QueueTimer& other);
    int getInfoSize() const;
    
    int getQueueId();
    snd_seq_queue_timer_type_t getType();
    const snd_timer_id_t* getId();
    unsigned int getResolution();
    void setType(snd_seq_queue_timer_type_t value);
    void setId(snd_timer_id_t* value);
    void setId(const TimerId& id);
    void setResolution(unsigned int value);

private:
    snd_seq_queue_timer_t* m_Info;
};

/**
 * Queue management.
 *
 * This class represents an ALSA sequencer queue object.
 */
class DRUMSTICK_EXPORT MidiQueue : public QObject
{
    Q_OBJECT
public:
    explicit MidiQueue(MidiClient* seq, QObject* parent = 0);
    MidiQueue(MidiClient* seq, const QueueInfo& info, QObject* parent = 0);
    MidiQueue(MidiClient* seq, const QString name, QObject* parent = 0);
    MidiQueue(MidiClient* seq, const int queue_id, QObject* parent = 0);
    virtual ~MidiQueue();

    int getId() const { return m_Id; }
    void start();
    void stop();
    void continueRunning();
    void clear();
    void setTickPosition(snd_seq_tick_time_t pos);
    void setRealTimePosition(snd_seq_real_time_t* pos);
    QueueInfo& getInfo();
    QueueStatus& getStatus();
    QueueTempo& getTempo();
    QueueTimer& getTimer();
    int getUsage();
    void setInfo(const QueueInfo& value);
    void setTempo(const QueueTempo& value);
    void setTimer(const QueueTimer& value);
    void setUsage(int used);

private:
    bool m_allocated;
    int m_Id;
    MidiClient* m_MidiClient;
    QueueInfo  m_Info;
    QueueTempo m_Tempo;
    QueueTimer m_Timer;
    QueueStatus m_Status;
};

} /* namespace drumstick */

/** @} */

#endif //DRUMSTICK_ALSAQUEUE_H
