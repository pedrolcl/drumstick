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

#include "alsaqueue.h"
#include "alsaclient.h"
#include "alsaevent.h"
#include "alsatimer.h"
#include <cmath>

/**
 * @file alsaqueue.cpp
 * Implementation of classes managing ALSA Sequencer queues
 */

namespace drumstick {

/**
 * @addtogroup ALSAQueue
 * @{
 *
 * ALSA events can be delivered to the output ports at scheduled times using the
 * queues. There is a small amount of available queues in the system, so this is
 * a limited resource. Queues are also used to time-stamp incoming events.
 *
 * Classes:
 *
 * QueueInfo holds several properties about the queue object.
 *
 * QueueStatus is used to retrieve the status of the queue object.
 *
 * QueueTempo holds properties to get and set the tempo in the queue object.
 *
 * QueueTimer holds properties about the Timer used in the queue object.
 *
 * MidiQueue represents the queue object.
 *
 * @see http://www.alsa-project.org/alsa-doc/alsa-lib/group___seq_queue.html
 * @}
 */

/**
 * Default constructor
 */
QueueInfo::QueueInfo()
{
    snd_seq_queue_info_malloc(&m_Info);
}

/**
 * Constructor.
 * @param other ALSA queue info object pointer
 */
QueueInfo::QueueInfo(snd_seq_queue_info_t* other)
{
    snd_seq_queue_info_malloc(&m_Info);
    snd_seq_queue_info_copy(m_Info, other);
}

/**
 * Copy constructor.
 * @param other An existing QueueInfo object reference.
 */
QueueInfo::QueueInfo(const QueueInfo& other)
{
    snd_seq_queue_info_malloc(&m_Info);
    snd_seq_queue_info_copy(m_Info, other.m_Info);
}

/**
 * Destructor
 */
QueueInfo::~QueueInfo()
{
    snd_seq_queue_info_free(m_Info);
}

/**
 * Copy the current object and return the copy.
 * @return The pointer to the new object.
 */
QueueInfo* QueueInfo::clone()
{
    return new QueueInfo(m_Info);
}

/**
 * Assignment operator.
 * @param other An existing QueueInfo object reference.
 * @return This object.
 */
QueueInfo& QueueInfo::operator=(const QueueInfo& other)
{
    snd_seq_queue_info_copy(m_Info, other.m_Info);
    return *this;
}

/**
 * Gets the queue's numeric identifier.
 * @return The numeric identifier.
 */
int QueueInfo::getId()
{
    return snd_seq_queue_info_get_queue(m_Info);
}

/**
 * Gets the queue name
 * @return The queue name.
 */
QString QueueInfo::getName()
{
    return QString(snd_seq_queue_info_get_name(m_Info));
}

/**
 * Gets the owner's client id of the queue.
 * @return the owner's client id.
 */
int QueueInfo::getOwner()
{
    return snd_seq_queue_info_get_owner(m_Info);
}

/**
 * Returns the locking status of the queue
 * @return The locking status.
 */
bool QueueInfo::isLocked()
{
    return (snd_seq_queue_info_get_locked(m_Info) != 0);
}

/**
 * Gets the flags of the queue.
 * @return The flags of the queue.
 */
unsigned int QueueInfo::getFlags()
{
    return snd_seq_queue_info_get_flags(m_Info);
}

/**
 * Sets the queue name
 * @param value The queue name
 */
void QueueInfo::setName(QString value)
{
    snd_seq_queue_info_set_name(m_Info, value.toLocal8Bit().data());
}

/**
 * Sets the client ID of the owner
 * @param value The client ID of the owner
 */
void QueueInfo::setOwner(int value)
{
    snd_seq_queue_info_set_owner(m_Info, value);
}

/**
 * Sets the bit flags of the queue
 * @param value The bit flags
 */
void QueueInfo::setFlags(unsigned int value)
{
    snd_seq_queue_info_set_flags(m_Info, value);
}

/**
 * Sets the locked status of the queue
 * @param locked The locked status
 */
void QueueInfo::setLocked(bool locked)
{
    snd_seq_queue_info_set_locked(m_Info, locked ? 1 : 0);
}

/**
 * Gets the size of the ALSA queue info object.
 * @return The size of the ALSA object.
 */
int QueueInfo::getInfoSize() const
{
    return snd_seq_queue_info_sizeof();
}


/**
 * Default constructor
 */
QueueStatus::QueueStatus()
{
    snd_seq_queue_status_malloc(&m_Info);
}

/**
 * Constructor
 * @param other ALSA queue status object pointer
 */
QueueStatus::QueueStatus(snd_seq_queue_status_t* other)
{
    snd_seq_queue_status_malloc(&m_Info);
    snd_seq_queue_status_copy(m_Info, other);
}

/**
 * Copy constructor
 * @param other An existing QueueStatus object reference
 */
QueueStatus::QueueStatus(const QueueStatus& other)
{
    snd_seq_queue_status_malloc(&m_Info);
    snd_seq_queue_status_copy(m_Info, other.m_Info);
}

/**
 * Destructor
 */
QueueStatus::~QueueStatus()
{
    snd_seq_queue_status_free(m_Info);
}

/**
 * Copy the current object and return the copy
 * @return The pointer to the new object
 */
QueueStatus* QueueStatus::clone()
{
    return new QueueStatus(m_Info);
}

/**
 * Assignment operator
 * @param other An existing QueueStatus object reference
 * @return This object
 */
QueueStatus& QueueStatus::operator=(const QueueStatus& other)
{
    snd_seq_queue_status_copy(m_Info, other.m_Info);
    return *this;
}

/**
 * Gets the queue's numeric identifier
 * @return The queue's numeric identifier.
 */
int QueueStatus::getId()
{
    return snd_seq_queue_status_get_queue(m_Info);
}

/**
 * Gets the number of queued events
 * @return The number of queued events
 */
int QueueStatus::getEvents()
{
    return snd_seq_queue_status_get_events(m_Info);
}

/**
 * Gets the real time (secods and nanoseconds) of the queue
 * @return The queue's real time.
 */
const snd_seq_real_time_t* QueueStatus::getRealtime()
{
    return snd_seq_queue_status_get_real_time(m_Info);
}

/**
 * Gets the running status bits
 * @return The running status bits
 */
unsigned int QueueStatus::getStatusBits()
{
    return snd_seq_queue_status_get_status(m_Info);
}

/**
 * Gets the musical time (ticks) of the queue
 * @return The musical time
 */
snd_seq_tick_time_t QueueStatus::getTickTime()
{
    return snd_seq_queue_status_get_tick_time(m_Info);
}

/**
 * Gets the size of the ALSA status object
 * @return The size of the ALSA object
 */
int QueueStatus::getInfoSize() const
{
    return snd_seq_queue_status_sizeof();
}

/**
 * Gets the queue's running state
 * @return True if the queue is running
 */
bool QueueStatus::isRunning()
{
    return (snd_seq_queue_status_get_status(m_Info) != 0);
}

/**
 * Gets the clock time in seconds of the queue
 * @return The queue time in seconds
 */
double QueueStatus::getClockTime()
{
    const snd_seq_real_time_t* time = snd_seq_queue_status_get_real_time(m_Info);
    return (time->tv_sec * 1.0) + (time->tv_nsec * 1.0e-9);
}

/**
 * Default constructor
 */
QueueTempo::QueueTempo()
{
    snd_seq_queue_tempo_malloc(&m_Info);
}

/**
 * Constructor
 * @param other An ALSA queue tempo object pointer
 */
QueueTempo::QueueTempo(snd_seq_queue_tempo_t* other)
{
    snd_seq_queue_tempo_malloc(&m_Info);
    snd_seq_queue_tempo_copy(m_Info, other);
}

/**
 * Copy constructor
 * @param other An existing QueueTempo object reference
 */
QueueTempo::QueueTempo(const QueueTempo& other)
{
    snd_seq_queue_tempo_malloc(&m_Info);
    snd_seq_queue_tempo_copy(m_Info, other.m_Info);
}

/**
 * Destructor
 */
QueueTempo::~QueueTempo()
{
    snd_seq_queue_tempo_free(m_Info);
}

/**
 * Copy the current object returning the copied object
 * @return The pointer to the new object
 */
QueueTempo* QueueTempo::clone()
{
    return new QueueTempo(m_Info);
}

/**
 * Assignment operator
 * @param other An existing QueueTempo object reference
 * @return This object
 */
QueueTempo& QueueTempo::operator=(const QueueTempo& other)
{
    snd_seq_queue_tempo_copy(m_Info, other.m_Info);
    return *this;
}

/**
 * Gets the queue's numeric identifier
 * @return The queue's numeric identifier
 */
int QueueTempo::getId()
{
    return snd_seq_queue_tempo_get_queue(m_Info);
}

/**
 * Gets the PPQ (parts per quarter note) resolution of the queue
 * @return The PPQ (parts per quarter note) resolution
 */
int QueueTempo::getPPQ()
{
    return snd_seq_queue_tempo_get_ppq(m_Info);
}

/**
 * Gets the tempo skew numerator. The real skew factor is the quotient of this
 * value divided by the skew base.
 * @return The tempo skew numerator.
 * @see getSkewBase(), setSkewValue(), setTempoFactor()
 */
unsigned int QueueTempo::getSkewValue()
{
    return snd_seq_queue_tempo_get_skew(m_Info);
}

/**
 * Gets the tempo skew base. The real skew factor is the quotient of the skew
 * value divided by the skew base.
 * @return The tempo skew base.
 * @see getSkewValue(), setSkewValue(), setTempoFactor()
 */
unsigned int QueueTempo::getSkewBase()
{
    return snd_seq_queue_tempo_get_skew_base(m_Info);
}

/**
 * Gets the queue's tempo in microseconds per beat.
 * @return The queue's tempo in microseconds per beat.
 */
unsigned int QueueTempo::getTempo()
{
    return snd_seq_queue_tempo_get_tempo(m_Info);
}

/**
 * Sets the queue resolution in parts per quarter note.
 * @param value The queue resolution in PPQ.
 */
void QueueTempo::setPPQ(int value)
{
    snd_seq_queue_tempo_set_ppq(m_Info, value);
}

/**
 * Sets the tempo skew numerator. The real skew factor is the quotient of this
 * value divided by the skew base.
 * @param value The tempo skew numerator.
 * @see getSkewBase(), getSkewValue(), setTempoFactor()
 */
void QueueTempo::setSkewValue(unsigned int value)
{
    snd_seq_queue_tempo_set_skew(m_Info, value);
}

/**
 * Sets the tempo skew base. The real skew factor is the quotient of the skew
 * value divided by the skew base.
 * @bug Protected because ALSA only accepts as argument a constant SKEW_BASE
 * @param value The tempo skew base.
 * @see getSkewBase(), getSkewValue(), setTempoFactor()
 */
void QueueTempo::setSkewBase(unsigned int value)
{
    snd_seq_queue_tempo_set_skew_base(m_Info, value);
}

/**
 * Sets the queue tempo in microseconds per beat
 * @param value The tempo in microseconds per beat
 */
void QueueTempo::setTempo(unsigned int value)
{
    snd_seq_queue_tempo_set_tempo(m_Info, value);
}

/**
 * Gets the queue's nominal BPM tempo (in beats per minute)
 * @return The queue's nominal BPM tempo (in beats per minute)
 */
float QueueTempo::getNominalBPM() 
{
    int itempo = getTempo();
    if (itempo != 0)
        return 6.0e7f / itempo;
    return 0.0f;
}

/**
 * Gets the queue's real BPM tempo in beats per minute. The result is equal to
 * the nominal BPM tempo multiplied by the skew factor.
 * @return
 */
float QueueTempo::getRealBPM() 
{
    float tempo = getNominalBPM();
    return tempo * getSkewValue() / SKEW_BASE;
}

/**
 * Sets the queue's tempo skew factor
 * @param value The tempo skew factor.
 */
void QueueTempo::setTempoFactor(float value) 
{
    setSkewValue(floor(SKEW_BASE * value));
    setSkewBase(SKEW_BASE);
}

/**
 * Sets the queue's nominal tempo in BPM (beats per minute).
 * @param value The nominal tempo in BPM (beats per minute).
 */
void QueueTempo::setNominalBPM(float value)
{
    setTempo(floor(6.0e7f / value));
}

/**
 * Gets the size of the ALSA queue tempo object
 * @return The size of the ALSA object
 */
int QueueTempo::getInfoSize() const
{
    return snd_seq_queue_tempo_sizeof();
}

/**
 * Default constructor
 */
QueueTimer::QueueTimer()
{
    snd_seq_queue_timer_malloc(&m_Info);
}

/**
 * Constructor
 * @param other An ALSA queue timer object pointer
 */
QueueTimer::QueueTimer(snd_seq_queue_timer_t* other)
{
    snd_seq_queue_timer_malloc(&m_Info);
    snd_seq_queue_timer_copy(m_Info, other);
}

/**
 * Copy constructor
 * @param other An existing QueueTimer object reference
 */
QueueTimer::QueueTimer(const QueueTimer& other)
{
    snd_seq_queue_timer_malloc(&m_Info);
    snd_seq_queue_timer_copy(m_Info, other.m_Info);
}

/**
 * Destructor
 */
QueueTimer::~QueueTimer()
{
    snd_seq_queue_timer_free(m_Info);
}

/**
 * Copy the current object and return the copy
 * @return The pointer to the new object
 */
QueueTimer* QueueTimer::clone()
{
    return new QueueTimer(m_Info);
}

/**
 * Assignment operator
 * @param other An existing QueueTimer object reference
 * @return This object
 */
QueueTimer& QueueTimer::operator=(const QueueTimer& other)
{
    snd_seq_queue_timer_copy(m_Info, other.m_Info);
    return *this;
}

/**
 * The queue's numeric identifier
 * @return The queue's numeric identifier
 */
int QueueTimer::getQueueId()
{
    return snd_seq_queue_timer_get_queue(m_Info);
}

/**
 * Gets the timer type.
 *
 * The timer type can be one of the following constants:
 * <ul>
 * <li>SND_SEQ_TIMER_ALSA: ALSA timer</li>
 * <li>SND_SEQ_TIMER_MIDI_CLOCK: MIDI Clock (CLOCK event)</li>
 * <li>SND_SEQ_TIMER_MIDI_TICK: MIDI Timer Tick (TICK event)</li>
 * </ul>
 * @return the timer type.
 * @see setType()
 */
snd_seq_queue_timer_type_t QueueTimer::getType()
{
    return snd_seq_queue_timer_get_type(m_Info);
}

/**
 * Gets the timer identifier record
 * @return The timer identifier record pointer
 */
const snd_timer_id_t* QueueTimer::getId()
{
    return snd_seq_queue_timer_get_id(m_Info);
}

/**
 * Gets the timer resolution
 * @return The timer resolution
 */
unsigned int QueueTimer::getResolution()
{
    return snd_seq_queue_timer_get_resolution(m_Info);
}

/**
 * Sets the timer type.
 * The timer type can be one of the following constants:
 * <ul>
 * <li>SND_SEQ_TIMER_ALSA: ALSA timer</li>
 * <li>SND_SEQ_TIMER_MIDI_CLOCK: MIDI Clock (CLOCK event)</li>
 * <li>SND_SEQ_TIMER_MIDI_TICK: MIDI Timer Tick (TICK event)</li>
 * </ul>
 * @param value The timer type
 * @see getType()
 */
void QueueTimer::setType(snd_seq_queue_timer_type_t value)
{
    snd_seq_queue_timer_set_type(m_Info, value);
}

/**
 * Sets the timer identifier record
 * @param value The timer identifier record pointer
 */
void QueueTimer::setId(snd_timer_id_t* value)
{
    snd_seq_queue_timer_set_id(m_Info, value);
}

/**
 * Sets the timer identifier record
 * @param id Timer identifier object
 */
void QueueTimer::setId(const TimerId& id)
{
    setId(id.m_Info);
}

/**
 * Sets the timer resolution
 * @param value The timer resolution
 */
void QueueTimer::setResolution(unsigned int value)
{
    snd_seq_queue_timer_set_resolution(m_Info, value);
}

/**
 * Gets the size of the ALSA queue timer object
 * @return The size of the ALSA object
 */
int QueueTimer::getInfoSize() const
{
    return snd_seq_queue_timer_sizeof();
}

/**
 * Constructor
 * @param seq An existing MidiClient instance
 * @param parent An optional parent object
 */
MidiQueue::MidiQueue(MidiClient* seq, QObject* parent) 
    : QObject(parent)
{
    m_MidiClient = seq;
    m_Id = CHECK_ERROR(snd_seq_alloc_queue(m_MidiClient->getHandle()));
    m_allocated = !(m_Id < 0);
}

/**
 * Constructor
 * @param seq An existing MidiClient instance
 * @param info A QueueInfo object reference
 * @param parent An optional parent object
 */
MidiQueue::MidiQueue(MidiClient* seq, const QueueInfo& info, QObject* parent)
    : QObject(parent)
{
    m_MidiClient = seq;
    m_Info = info;
    m_Id = CHECK_ERROR(snd_seq_create_queue(m_MidiClient->getHandle(), m_Info.m_Info));
    m_allocated = !(m_Id < 0);
}

/**
 * Constructor
 * @param seq An existing MidiClient instance
 * @param name The name for the new queue
 * @param parent An optional parent object
 */
MidiQueue::MidiQueue(MidiClient* seq, const QString name, QObject* parent)
    : QObject(parent)
{
    m_MidiClient = seq;
    m_Id = CHECK_ERROR(snd_seq_alloc_named_queue(m_MidiClient->getHandle(), name.toLocal8Bit().data()));
    m_allocated = !(m_Id < 0);
}

/**
 * Constructor.
 *
 * Note: this constructor doesn't allocate a new queue, it uses an existing one.
 * @param seq An existing MidiClient instance
 * @param queue_id An existing queue numeric identifier
 * @param parent An optional parent object
 */
MidiQueue::MidiQueue(MidiClient* seq, const int queue_id, QObject* parent)
    : QObject(parent)
{
    m_MidiClient = seq;
    m_Id = queue_id;
    m_allocated = false;
}

/**
 * Destructor
 */
MidiQueue::~MidiQueue()
{
    if ( m_allocated && (m_MidiClient->getHandle() != NULL) )
    {
        CHECK_ERROR(snd_seq_free_queue(m_MidiClient->getHandle(), m_Id));
    }
}

/**
 * Gets a QueueInfo object reference
 * @return A QueueInfo object reference
 */
QueueInfo& MidiQueue::getInfo()
{
    CHECK_WARNING(snd_seq_get_queue_info(m_MidiClient->getHandle(), m_Id, m_Info.m_Info));
    return m_Info;
}

/**
 * Gets a QueueStatus object reference
 * @return A QueueStatus object reference
 */
QueueStatus& MidiQueue::getStatus()
{
    CHECK_WARNING(snd_seq_get_queue_status(m_MidiClient->getHandle(), m_Id, m_Status.m_Info));
    return m_Status;
}

/**
 * Gets a QueueTempo object reference
 * @return A QueueTempo object reference
 */
QueueTempo& MidiQueue::getTempo()
{
    CHECK_WARNING(snd_seq_get_queue_tempo(m_MidiClient->getHandle(), m_Id, m_Tempo.m_Info));
    return m_Tempo;
}

/**
 * Gets a QueueTimer object reference
 * @return A QueueTimer object reference
 */
QueueTimer& MidiQueue::getTimer()
{
    CHECK_WARNING(snd_seq_get_queue_timer(m_MidiClient->getHandle(), m_Id, m_Timer.m_Info));
    return m_Timer;
}

/**
 * Applies a QueueInfo object to the queue
 * @param value A QueueInfo object reference
 */
void MidiQueue::setInfo(const QueueInfo& value)
{
    m_Info = value;
    CHECK_WARNING(snd_seq_set_queue_info(m_MidiClient->getHandle(), m_Id, m_Info.m_Info));
}

/**
 * Applies a QueueTempo object to the queue
 * @param value A QueueTempo object reference
 */
void MidiQueue::setTempo(const QueueTempo& value)
{
    m_Tempo = value;
    CHECK_WARNING(snd_seq_set_queue_tempo(m_MidiClient->getHandle(), m_Id, m_Tempo.m_Info));
}

/**
 * Applies q QueueTimer object to the queue
 * @param value A QueueTimer object reference
 */
void MidiQueue::setTimer(const QueueTimer& value)
{
    m_Timer = value;
    CHECK_WARNING(snd_seq_set_queue_timer(m_MidiClient->getHandle(), m_Id, m_Timer.m_Info));
}

/**
 * Gets the queue usage flag.
 *
 * @return 1 = client is allowed to access the queue, 0 = not allowed.
 */
int MidiQueue::getUsage()
{
    return CHECK_WARNING(snd_seq_get_queue_usage(m_MidiClient->getHandle(), m_Id));
}

/**
 * Sets the queue usage flag.
 *
 * @param used 1 = client is allowed to access the queue, 0 = not allowed.
 */
void MidiQueue::setUsage(int used)
{
    CHECK_WARNING(snd_seq_set_queue_usage(m_MidiClient->getHandle(), m_Id, used));
}

/**
 * Start the queue.
 *
 * This method should start running the queue from the initial position.
 */
void MidiQueue::start()
{
    CHECK_WARNING(snd_seq_start_queue(m_MidiClient->getHandle(), m_Id, NULL));
    CHECK_WARNING(snd_seq_drain_output(m_MidiClient->getHandle()));
}

/**
 * Stop the queue.
 *
 * This method should stop running the queue.
 */
void MidiQueue::stop()
{
    if (m_MidiClient != NULL && m_MidiClient->getHandle() != NULL) {
        CHECK_WARNING(snd_seq_stop_queue(m_MidiClient->getHandle(), m_Id, NULL));
        CHECK_WARNING(snd_seq_drain_output(m_MidiClient->getHandle()));
    }
}

/**
 * Start the queue without resetting the last position.
 *
 * This method should start running the queue from the last position set.
 */
void MidiQueue::continueRunning()
{
    CHECK_WARNING(snd_seq_continue_queue(m_MidiClient->getHandle(), m_Id, NULL));
    CHECK_WARNING(snd_seq_drain_output(m_MidiClient->getHandle()));
}

/**
 * Clear the queue, dropping any scheduled events.
 */
void MidiQueue::clear()
{
    if (m_MidiClient != NULL && m_MidiClient->getHandle() != NULL)
        snd_seq_drop_output(m_MidiClient->getHandle());
}

/**
 * Sets the queue position in musical time (ticks).
 * @param pos Musical time in ticks.
 */
void MidiQueue::setTickPosition(snd_seq_tick_time_t pos)
{
    SystemEvent event(SND_SEQ_EVENT_SETPOS_TICK);
    snd_seq_ev_set_queue_pos_tick(event.getHandle(), m_Id, pos);
    event.setDirect();
    m_MidiClient->outputDirect(&event);
}

/**
 * Sets the queue position in real time (clock) units: seconds and nanoseconds.
 * @param pos Real time (clock) position in seconds/nanoseconds.
 */
void MidiQueue::setRealTimePosition(snd_seq_real_time_t* pos)
{
    SystemEvent event(SND_SEQ_EVENT_SETPOS_TIME);
    snd_seq_ev_set_queue_pos_real(event.getHandle(), m_Id, pos);
    event.setDirect();
    m_MidiClient->outputDirect(&event);
}

} /* namespace drumstick */
