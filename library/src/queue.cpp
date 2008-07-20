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
#include "queue.h"
#include "client.h"
#include "event.h"

namespace ALSA
{
namespace Sequencer
{

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

QueueInfo::~QueueInfo()
{
    snd_seq_queue_info_free(m_Info);
}

QueueInfo* QueueInfo::clone()
{
    return new QueueInfo(m_Info);
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

bool QueueInfo::getLocked()
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

QueueStatus::~QueueStatus()
{
    snd_seq_queue_status_free(m_Info);
}

QueueStatus* QueueStatus::clone()
{
    return new QueueStatus(m_Info);
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

QueueTempo::~QueueTempo()
{
    snd_seq_queue_tempo_free(m_Info);
}

QueueTempo* QueueTempo::clone()
{
    return new QueueTempo(m_Info);
}

int QueueTempo::getId()
{
    return snd_seq_queue_tempo_get_queue(m_Info);
}

int QueueTempo::getPPQ()
{
    return snd_seq_queue_tempo_get_ppq(m_Info);
}

unsigned int QueueTempo::getSkew()
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

void QueueTempo::setSkew(unsigned int value)
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

QueueTimer::~QueueTimer()
{
    snd_seq_queue_timer_free(m_Info);
}

QueueTimer* QueueTimer::clone()
{
    return new QueueTimer(m_Info);
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

/*********/
/* MidiQueue */
/*********/

MidiQueue::MidiQueue(MidiClient* seq) :
    m_MidiClient(NULL), m_Info(NULL), m_Tempo(NULL), m_Timer(NULL)
{
    m_MidiClient = seq;
    m_Id = CHECK_ERROR(snd_seq_alloc_queue(m_MidiClient->getHandle()));
}

MidiQueue::MidiQueue(MidiClient* seq, QueueInfo* info)
{
    m_MidiClient = seq;
    m_Info = info;
    m_Id = CHECK_ERROR(snd_seq_create_queue(m_MidiClient->getHandle(), m_Info->m_Info));
}

MidiQueue::MidiQueue(MidiClient* seq, QString name)
{
    m_MidiClient = seq;
    m_Id = CHECK_ERROR(snd_seq_alloc_named_queue(m_MidiClient->getHandle(), name.toLocal8Bit().data()));
}

MidiQueue::~MidiQueue()
{
    if (m_MidiClient->getHandle() != NULL)
    {
        CHECK_ERROR(snd_seq_free_queue(m_MidiClient->getHandle(), m_Id));
    }
    if (m_Tempo != NULL)
        delete m_Tempo;
    if (m_Timer != NULL)
        delete m_Timer;
    if (m_Info != NULL)
        delete m_Info;
}

QueueInfo* MidiQueue::getInfo()
{
    if (m_Info == NULL)
    {
        snd_seq_queue_info_t* info;
        snd_seq_queue_info_alloca(&info);
        CHECK_EXCEPT(snd_seq_get_queue_info(m_MidiClient->getHandle(), m_Id, info));
        m_Info = new QueueInfo(info);
    }
    return m_Info;
}

QueueStatus* MidiQueue::getStatus()
{
    snd_seq_queue_status_t* info;
    snd_seq_queue_status_alloca(&info);
    CHECK_EXCEPT(snd_seq_get_queue_status(m_MidiClient->getHandle(), m_Id, info));
    return new QueueStatus(info);
}

QueueTempo* MidiQueue::getTempo()
{
    if (m_Tempo == NULL) {
        snd_seq_queue_tempo_t* info;
        snd_seq_queue_tempo_alloca(&info);
        CHECK_EXCEPT(snd_seq_get_queue_tempo(m_MidiClient->getHandle(), m_Id, info));
        m_Tempo = new QueueTempo(info);
    }
    return m_Tempo;
}

QueueTimer* MidiQueue::getTimer()
{
    if (m_Timer == NULL) {
        snd_seq_queue_timer_t* info;
        snd_seq_queue_timer_alloca(&info);
        CHECK_EXCEPT(snd_seq_get_queue_timer(m_MidiClient->getHandle(), m_Id, info));
        m_Timer = new QueueTimer(info);
    }
    return m_Timer;
}

void MidiQueue::setInfo(QueueInfo* value)
{
    CHECK_EXCEPT(snd_seq_set_queue_info(m_MidiClient->getHandle(), m_Id, value->m_Info));
    m_Info = value;
}

void MidiQueue::setTempo(QueueTempo* value)
{
    CHECK_EXCEPT(snd_seq_set_queue_tempo(m_MidiClient->getHandle(), m_Id, value->m_Info));
    m_Tempo = value;
}

void MidiQueue::setTimer(QueueTimer* value)
{
    CHECK_EXCEPT(snd_seq_set_queue_timer(m_MidiClient->getHandle(), m_Id, value->m_Info));
    m_Timer = value;
}

int MidiQueue::getUsage()
{
    return CHECK_EXCEPT(snd_seq_get_queue_usage(m_MidiClient->getHandle(), m_Id));
}

void MidiQueue::setUsage(int used)
{
    CHECK_EXCEPT(snd_seq_set_queue_usage(m_MidiClient->getHandle(), m_Id, used));
}

void MidiQueue::start()
{
    CHECK_EXCEPT(snd_seq_start_queue(m_MidiClient->getHandle(), m_Id, NULL));
    CHECK_EXCEPT(snd_seq_drain_output(m_MidiClient->getHandle()));
}

void MidiQueue::stop()
{
    CHECK_EXCEPT(snd_seq_stop_queue(m_MidiClient->getHandle(), m_Id, NULL));
    CHECK_EXCEPT(snd_seq_drain_output(m_MidiClient->getHandle()));
}

void MidiQueue::continueRunning()
{
    CHECK_EXCEPT(snd_seq_continue_queue(m_MidiClient->getHandle(), m_Id, NULL));
    CHECK_EXCEPT(snd_seq_drain_output(m_MidiClient->getHandle()));
}

void MidiQueue::clear()
{
    snd_seq_drop_output(m_MidiClient->getHandle());
}

void MidiQueue::setTickPosition(snd_seq_tick_time_t pos)
{
    SystemEvent* event = new SystemEvent(SND_SEQ_EVENT_SETPOS_TICK);
    snd_seq_ev_set_queue_pos_tick(event->getEvent(), m_Id, pos);
    event->setDirect();
    m_MidiClient->outputDirect(event);
    m_MidiClient->drainOutput();
}

void MidiQueue::setRealTimePosition(snd_seq_real_time_t* pos)
{
    SystemEvent* event = new SystemEvent(SND_SEQ_EVENT_SETPOS_TIME);
    snd_seq_ev_set_queue_pos_real(event->getEvent(), m_Id, pos);
    event->setDirect();
    m_MidiClient->outputDirect(event);
    m_MidiClient->drainOutput();
}

}
}
