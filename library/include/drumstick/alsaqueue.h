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
 
#ifndef DRUMSTICK_ALSAQUEUE_H
#define DRUMSTICK_ALSAQUEUE_H

#include <QObject>

extern "C" {
    #include <alsa/asoundlib.h>
}

#include "macros.h"

namespace drumstick { namespace ALSA {

/**
 * @file alsaqueue.h
 * Classes managing ALSA Sequencer queues
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
class TimerId;

/**
 * @addtogroup ALSAQueue ALSA Sequencer Queues
 * @{
 *
 * @class QueueInfo
 * Queue information container.
 *
 * This class is used to hold some properties about an ALSA queue object.
 */
class DRUMSTICK_ALSA_EXPORT QueueInfo
{
    friend class MidiQueue;

public:
    QueueInfo();
    QueueInfo(const QueueInfo& other);
    explicit QueueInfo(snd_seq_queue_info_t* other);
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
class DRUMSTICK_ALSA_EXPORT QueueStatus
{
    friend class MidiQueue;

public:
    QueueStatus();
    QueueStatus(const QueueStatus& other);
    explicit QueueStatus(snd_seq_queue_status_t* other);
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
class DRUMSTICK_ALSA_EXPORT QueueTempo
{
    friend class MidiQueue;

public:
    QueueTempo();
    QueueTempo(const QueueTempo& other);
    explicit QueueTempo(snd_seq_queue_tempo_t* other);
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
class DRUMSTICK_ALSA_EXPORT QueueTimer
{
    friend class MidiQueue;

public:
    QueueTimer();
    QueueTimer(const QueueTimer& other);
    explicit QueueTimer(snd_seq_queue_timer_t* other);
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
class DRUMSTICK_ALSA_EXPORT MidiQueue : public QObject
{
    Q_OBJECT
public:
    explicit MidiQueue(MidiClient* seq, QObject* parent = nullptr);
    MidiQueue(MidiClient* seq, const QueueInfo& info, QObject* parent = nullptr);
    MidiQueue(MidiClient* seq, const QString name, QObject* parent = nullptr);
    MidiQueue(MidiClient* seq, const int queue_id, QObject* parent = nullptr);
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

/** @} */

}} /* namespace drumstick::ALSA */

#endif //DRUMSTICK_ALSAQUEUE_H
