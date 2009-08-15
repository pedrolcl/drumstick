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

#include "alsaqueue.h"
#include "alsaclient.h"
#include "alsaevent.h"
#include <cmath>

namespace aseqmm {

/*************/
/* QueueInfo */
/*************/

QueueInfo::QueueInfo()
{
    snd_seq_queue_info_malloc(&m_Info);
}

QueueInfo::QueueInfo(snd_seq_queue_info_t* other)
{
    snd_seq_queue_info_malloc(&m_Info);
    snd_seq_queue_info_copy(m_Info, other);
}

QueueInfo::QueueInfo(const QueueInfo& other)
{
    snd_seq_queue_info_malloc(&m_Info);
    snd_seq_queue_info_copy(m_Info, other.m_Info);
}

QueueInfo::~QueueInfo()
{
    snd_seq_queue_info_free(m_Info);
}

QueueInfo* QueueInfo::clone()
{
    return new QueueInfo(m_Info);
}

QueueInfo& QueueInfo::operator=(const QueueInfo& other)
{
    snd_seq_queue_info_copy(m_Info, other.m_Info);
    return *this;
}

int QueueInfo::getId()
{
    return snd_seq_queue_info_get_queue(m_Info);
}

QString QueueInfo::getName()
{
    return QString(snd_seq_queue_info_get_name(m_Info));
}

int QueueInfo::getOwner()
{
    return snd_seq_queue_info_get_owner(m_Info);
}

bool QueueInfo::isLocked()
{
    return (snd_seq_queue_info_get_locked(m_Info) != 0);
}

unsigned int QueueInfo::getFlags()
{
    return snd_seq_queue_info_get_flags(m_Info);
}

void QueueInfo::setName(QString value)
{
    snd_seq_queue_info_set_name(m_Info, value.toLocal8Bit().data());
}

void QueueInfo::setOwner(int value)
{
    snd_seq_queue_info_set_owner(m_Info, value);
}

void QueueInfo::setFlags(unsigned int value)
{
    snd_seq_queue_info_set_flags(m_Info, value);
}

void QueueInfo::setLocked(bool locked)
{
    snd_seq_queue_info_set_locked(m_Info, locked ? 1 : 0);
}

int QueueInfo::getInfoSize() const
{
    return snd_seq_queue_info_sizeof();
}

/***************/
/* QueueStatus */
/***************/

QueueStatus::QueueStatus()
{
    snd_seq_queue_status_malloc(&m_Info);
}

QueueStatus::QueueStatus(snd_seq_queue_status_t* other)
{
    snd_seq_queue_status_malloc(&m_Info);
    snd_seq_queue_status_copy(m_Info, other);
}

QueueStatus::QueueStatus(const QueueStatus& other)
{
    snd_seq_queue_status_malloc(&m_Info);
    snd_seq_queue_status_copy(m_Info, other.m_Info);
}

QueueStatus::~QueueStatus()
{
    snd_seq_queue_status_free(m_Info);
}

QueueStatus* QueueStatus::clone()
{
    return new QueueStatus(m_Info);
}

QueueStatus& QueueStatus::operator=(const QueueStatus& other)
{
    snd_seq_queue_status_copy(m_Info, other.m_Info);
    return *this;
}

int QueueStatus::getId()
{
    return snd_seq_queue_status_get_queue(m_Info);
}

int QueueStatus::getEvents()
{
    return snd_seq_queue_status_get_events(m_Info);
}

const snd_seq_real_time_t* QueueStatus::getRealtime()
{
    return snd_seq_queue_status_get_real_time(m_Info);
}

unsigned int QueueStatus::getStatusBits()
{
    return snd_seq_queue_status_get_status(m_Info);
}

snd_seq_tick_time_t QueueStatus::getTickTime()
{
    return snd_seq_queue_status_get_tick_time(m_Info);
}

int QueueStatus::getInfoSize() const
{
    return snd_seq_queue_status_sizeof();
}

bool QueueStatus::isRunning()
{
    return (snd_seq_queue_status_get_status(m_Info) != 0);
}

double QueueStatus::getClockTime()
{
    const snd_seq_real_time_t* time = snd_seq_queue_status_get_real_time(m_Info);
    return (time->tv_sec * 1.0) + (time->tv_nsec * 1.0e-9);
}

/**************/
/* QueueTempo */
/**************/

QueueTempo::QueueTempo()
{
    snd_seq_queue_tempo_malloc(&m_Info);
}

QueueTempo::QueueTempo(snd_seq_queue_tempo_t* other)
{
    snd_seq_queue_tempo_malloc(&m_Info);
    snd_seq_queue_tempo_copy(m_Info, other);
}

QueueTempo::QueueTempo(const QueueTempo& other)
{
    snd_seq_queue_tempo_malloc(&m_Info);
    snd_seq_queue_tempo_copy(m_Info, other.m_Info);
}

QueueTempo::~QueueTempo()
{
    snd_seq_queue_tempo_free(m_Info);
}

QueueTempo* QueueTempo::clone()
{
    return new QueueTempo(m_Info);
}

QueueTempo& QueueTempo::operator=(const QueueTempo& other)
{
    snd_seq_queue_tempo_copy(m_Info, other.m_Info);
    return *this;
}

int QueueTempo::getId()
{
    return snd_seq_queue_tempo_get_queue(m_Info);
}

int QueueTempo::getPPQ()
{
    return snd_seq_queue_tempo_get_ppq(m_Info);
}

unsigned int QueueTempo::getSkewValue()
{
    return snd_seq_queue_tempo_get_skew(m_Info);
}

unsigned int QueueTempo::getSkewBase()
{
    return snd_seq_queue_tempo_get_skew_base(m_Info);
}

unsigned int QueueTempo::getTempo()
{
    return snd_seq_queue_tempo_get_tempo(m_Info);
}

void QueueTempo::setPPQ(int value)
{
    snd_seq_queue_tempo_set_ppq(m_Info, value);
}

void QueueTempo::setSkewValue(unsigned int value)
{
    snd_seq_queue_tempo_set_skew(m_Info, value);
}

void QueueTempo::setSkewBase(unsigned int value)
{
    snd_seq_queue_tempo_set_skew_base(m_Info, value);
}

void QueueTempo::setTempo(unsigned int value)
{
    snd_seq_queue_tempo_set_tempo(m_Info, value);
}

float QueueTempo::getNominalBPM() 
{
    int itempo = getTempo();
    if (itempo != 0)
        return 6.0e7f / itempo;
    return 0.0f;
}

float QueueTempo::getRealBPM() 
{
    float tempo = getNominalBPM();
    return tempo * getSkewValue() / SKEW_BASE;
}

void QueueTempo::setTempoFactor(float value) 
{
    setSkewValue(floor(SKEW_BASE * value));
    setSkewBase(SKEW_BASE);
}

void QueueTempo::setNominalBPM(float value)
{
    setTempo(floor(6.0e7f / value));
}

int QueueTempo::getInfoSize() const
{
    return snd_seq_queue_tempo_sizeof();
}

/**************/
/* QueueTimer */
/**************/

QueueTimer::QueueTimer()
{
    snd_seq_queue_timer_malloc(&m_Info);
}

QueueTimer::QueueTimer(snd_seq_queue_timer_t* other)
{
    snd_seq_queue_timer_malloc(&m_Info);
    snd_seq_queue_timer_copy(m_Info, other);
}

QueueTimer::QueueTimer(const QueueTimer& other)
{
    snd_seq_queue_timer_malloc(&m_Info);
    snd_seq_queue_timer_copy(m_Info, other.m_Info);
}

QueueTimer::~QueueTimer()
{
    snd_seq_queue_timer_free(m_Info);
}

QueueTimer* QueueTimer::clone()
{
    return new QueueTimer(m_Info);
}

QueueTimer& QueueTimer::operator=(const QueueTimer& other)
{
    snd_seq_queue_timer_copy(m_Info, other.m_Info);
    return *this;
}

int QueueTimer::getQueueId()
{
    return snd_seq_queue_timer_get_queue(m_Info);
}

snd_seq_queue_timer_type_t QueueTimer::getType()
{
    return snd_seq_queue_timer_get_type(m_Info);
}

const snd_timer_id_t* QueueTimer::getId()
{
    return snd_seq_queue_timer_get_id(m_Info);
}

unsigned int QueueTimer::getResolution()
{
    return snd_seq_queue_timer_get_resolution(m_Info);
}

void QueueTimer::setType(snd_seq_queue_timer_type_t value)
{
    snd_seq_queue_timer_set_type(m_Info, value);
}

void QueueTimer::setId(snd_timer_id_t* value)
{
    snd_seq_queue_timer_set_id(m_Info, value);
}

void QueueTimer::setResolution(unsigned int value)
{
    snd_seq_queue_timer_set_resolution(m_Info, value);
}

int QueueTimer::getInfoSize() const
{
    return snd_seq_queue_timer_sizeof();
}

/*************/
/* MidiQueue */
/*************/

MidiQueue::MidiQueue(MidiClient* seq, QObject* parent) 
    : QObject(parent)
{
    m_MidiClient = seq;
    m_Id = CHECK_ERROR(snd_seq_alloc_queue(m_MidiClient->getHandle()));
    m_allocated = !(m_Id < 0);
}

MidiQueue::MidiQueue(MidiClient* seq, const QueueInfo info, QObject* parent)
    : QObject(parent)
{
    m_MidiClient = seq;
    m_Info = info;
    m_Id = CHECK_ERROR(snd_seq_create_queue(m_MidiClient->getHandle(), m_Info.m_Info));
    m_allocated = !(m_Id < 0);
}

MidiQueue::MidiQueue(MidiClient* seq, const QString name, QObject* parent)
    : QObject(parent)
{
    m_MidiClient = seq;
    m_Id = CHECK_ERROR(snd_seq_alloc_named_queue(m_MidiClient->getHandle(), name.toLocal8Bit().data()));
    m_allocated = !(m_Id < 0);
}

MidiQueue::MidiQueue(MidiClient* seq, const int queue_id, QObject* parent)
    : QObject(parent)
{
    m_MidiClient = seq;
    m_Id = queue_id;
    m_allocated = false;
}

MidiQueue::~MidiQueue()
{
    if ( m_allocated && (m_MidiClient->getHandle() != NULL) )
    {
        CHECK_ERROR(snd_seq_free_queue(m_MidiClient->getHandle(), m_Id));
    }
}

QueueInfo& MidiQueue::getInfo()
{
    CHECK_WARNING(snd_seq_get_queue_info(m_MidiClient->getHandle(), m_Id, m_Info.m_Info));
    return m_Info;
}

QueueStatus& MidiQueue::getStatus()
{
    CHECK_WARNING(snd_seq_get_queue_status(m_MidiClient->getHandle(), m_Id, m_Status.m_Info));
    return m_Status;
}

QueueTempo& MidiQueue::getTempo()
{
    CHECK_WARNING(snd_seq_get_queue_tempo(m_MidiClient->getHandle(), m_Id, m_Tempo.m_Info));
    return m_Tempo;
}

QueueTimer& MidiQueue::getTimer()
{
    CHECK_WARNING(snd_seq_get_queue_timer(m_MidiClient->getHandle(), m_Id, m_Timer.m_Info));
    return m_Timer;
}

void MidiQueue::setInfo(const QueueInfo& value)
{
    m_Info = value;
    CHECK_WARNING(snd_seq_set_queue_info(m_MidiClient->getHandle(), m_Id, m_Info.m_Info));
}

void MidiQueue::setTempo(const QueueTempo& value)
{
    m_Tempo = value;
    CHECK_WARNING(snd_seq_set_queue_tempo(m_MidiClient->getHandle(), m_Id, m_Tempo.m_Info));
}

void MidiQueue::setTimer(const QueueTimer& value)
{
    m_Timer = value;
    CHECK_WARNING(snd_seq_set_queue_timer(m_MidiClient->getHandle(), m_Id, m_Timer.m_Info));
}

int MidiQueue::getUsage()
{
    return CHECK_WARNING(snd_seq_get_queue_usage(m_MidiClient->getHandle(), m_Id));
}

void MidiQueue::setUsage(int used)
{
    CHECK_WARNING(snd_seq_set_queue_usage(m_MidiClient->getHandle(), m_Id, used));
}

void MidiQueue::start()
{
    CHECK_WARNING(snd_seq_start_queue(m_MidiClient->getHandle(), m_Id, NULL));
    CHECK_WARNING(snd_seq_drain_output(m_MidiClient->getHandle()));
}

void MidiQueue::stop()
{
    CHECK_WARNING(snd_seq_stop_queue(m_MidiClient->getHandle(), m_Id, NULL));
    CHECK_WARNING(snd_seq_drain_output(m_MidiClient->getHandle()));
}

void MidiQueue::continueRunning()
{
    CHECK_WARNING(snd_seq_continue_queue(m_MidiClient->getHandle(), m_Id, NULL));
    CHECK_WARNING(snd_seq_drain_output(m_MidiClient->getHandle()));
}

void MidiQueue::clear()
{
    snd_seq_drop_output(m_MidiClient->getHandle());
}

void MidiQueue::setTickPosition(snd_seq_tick_time_t pos)
{
    SystemEvent* event = new SystemEvent(SND_SEQ_EVENT_SETPOS_TICK);
    snd_seq_ev_set_queue_pos_tick(event->getHandle(), m_Id, pos);
    event->setDirect();
    m_MidiClient->outputDirect(event);
    m_MidiClient->drainOutput();
}

void MidiQueue::setRealTimePosition(snd_seq_real_time_t* pos)
{
    SystemEvent* event = new SystemEvent(SND_SEQ_EVENT_SETPOS_TIME);
    snd_seq_ev_set_queue_pos_real(event->getHandle(), m_Id, pos);
    event->setDirect();
    m_MidiClient->outputDirect(event);
    m_MidiClient->drainOutput();
}

}
