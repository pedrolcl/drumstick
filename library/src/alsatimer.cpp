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

#include "alsatimer.h"

namespace ALSA
{

using namespace Sequencer;

TimerInfo::TimerInfo()
{
    snd_timer_info_malloc(&m_Info);
}

TimerInfo::TimerInfo(const snd_timer_info_t *other)
{
    snd_timer_info_malloc(&m_Info);
    snd_timer_info_copy(m_Info, other);
}

TimerInfo::TimerInfo(const TimerInfo& other)
{
    snd_timer_info_malloc(&m_Info);
    snd_timer_info_copy(m_Info, other.m_Info);
}

TimerInfo::~TimerInfo()
{
    snd_timer_info_free(m_Info);
}

TimerInfo* 
TimerInfo::clone()
{
    return new TimerInfo(m_Info);
}

TimerInfo& 
TimerInfo::operator=(const TimerInfo& other)
{
    snd_timer_info_copy(m_Info, other.m_Info);
    return *this;
}

bool 
TimerInfo::isSlave()
{
    return (snd_timer_info_is_slave(m_Info) != 0);
}

int 
TimerInfo::getCard()
{
    return snd_timer_info_get_card(m_Info);
}

QString 
TimerInfo::getId()
{
    return QString(snd_timer_info_get_id(m_Info));
}

QString 
TimerInfo::getName()
{
    return QString(snd_timer_info_get_name(m_Info));
}

long 
TimerInfo::getResolution()
{
    return snd_timer_info_get_resolution(m_Info);
}

long 
TimerInfo::getFrequency()
{
    long res = getResolution();
    if (res > 0)
    {
        return 1000000000 / res;
    }
    return 0;
}

int 
TimerInfo::getSizeOfInfo() const
{
    return snd_timer_info_sizeof(); 
}

long
TimerInfo::getTicks()
{
    return snd_timer_info_get_ticks(m_Info);
}

/***********
 * TimerId *
 ***********/

TimerId::TimerId()
{
    snd_timer_id_malloc(&m_Info);
}

TimerId::TimerId(const snd_timer_id_t *other)
{
    snd_timer_id_malloc(&m_Info);
    snd_timer_id_copy(m_Info, other);
    if (getCard() < 0)
        setCard(0);
    if (getDevice() < 0)
        setDevice(0);
    if (getSubdevice() < 0)
        setSubdevice(0);
}

TimerId::TimerId(const TimerId& other)
{
    snd_timer_id_malloc(&m_Info);
    snd_timer_id_copy(m_Info, other.m_Info);
    if (getCard() < 0)
        setCard(0);
    if (getDevice() < 0)
        setDevice(0);
    if (getSubdevice() < 0)
        setSubdevice(0);
}

TimerId::TimerId(int cls, int scls, int card, int dev, int sdev)
{
    snd_timer_id_malloc(&m_Info);
    setClass(cls);
    setSlaveClass(scls);
    setCard(card);
    setDevice(dev);
    setSubdevice(sdev);
}

TimerId::~TimerId()
{
    snd_timer_id_free(m_Info);
}

TimerId* 
TimerId::clone()
{
    return new TimerId(m_Info);
}

TimerId& 
TimerId::operator=(const TimerId& other)
{
    snd_timer_id_copy(m_Info, other.m_Info);
    if (getCard() < 0)
    setCard(0);
    if (getDevice() < 0)
    setDevice(0);
    if (getSubdevice() < 0)
    setSubdevice(0);
    return *this;
}

void
TimerId::setClass(int devclass)
{
    snd_timer_id_set_class(m_Info, devclass);
}

int
TimerId::getClass()
{
    return snd_timer_id_get_class(m_Info);
}

void
TimerId::setSlaveClass(int devsclass)
{
    snd_timer_id_set_sclass(m_Info, devsclass);
}

int
TimerId::getSlaveClass()
{
    return snd_timer_id_get_sclass(m_Info);
}

void
TimerId::setCard(int card)
{
    snd_timer_id_set_card(m_Info, card);
}

int
TimerId::getCard()
{
    return snd_timer_id_get_card(m_Info);
}

void
TimerId::setDevice(int device)
{
    snd_timer_id_set_device(m_Info, device);
}

int
TimerId::getDevice()
{
    return snd_timer_id_get_device(m_Info);
}

void
TimerId::setSubdevice(int subdevice)
{
    snd_timer_id_set_subdevice (m_Info, subdevice);
}

int
TimerId::getSubdevice()
{
    return snd_timer_id_get_subdevice(m_Info);
}

int 
TimerId::getSizeOfInfo() const
{
    return snd_timer_id_sizeof(); 
}

/**************
 * TimerQuery *
 **************/

TimerQuery::TimerQuery(const QString& deviceName, int openMode)
{
    CHECK_WARNING( snd_timer_query_open( &m_Info, 
                                         deviceName.toLocal8Bit().data(), 
                                         openMode ));
    readTimers();
}

TimerQuery::TimerQuery( const QString& deviceName, int openMode, 
                        snd_config_t* conf )
{
    CHECK_WARNING( snd_timer_query_open_lconf( &m_Info, 
                                               deviceName.toLocal8Bit().data(), 
                                               openMode, conf ));
    readTimers();
}

TimerQuery::~TimerQuery()
{
    freeTimers();
    snd_timer_query_close(m_Info);
}

void
TimerQuery::readTimers()
{
    TimerId tid; 
    snd_timer_id_set_class(tid.m_Info, SND_TIMER_CLASS_NONE);
    for(;;)
    {
        int rc = snd_timer_query_next_device(m_Info, tid.m_Info);
        if ((rc < 0) || (tid.getClass() < 0)) {
            break;
        }
        m_timers.append(tid);
    }
}

void
TimerQuery::freeTimers()
{
    m_timers.clear();
}

TimerGlobalInfo&
TimerQuery::getGlobalInfo()
{
    snd_timer_query_info(m_Info, m_GlobalInfo.m_Info);
    return m_GlobalInfo;
}

void
TimerQuery::setGlobalParams(snd_timer_gparams_t* params)
{
    snd_timer_query_params(m_Info, params);
}

void
TimerQuery::getGlobalParams(snd_timer_gparams_t* params)
{
    snd_timer_query_params(m_Info, params);
}

void
TimerQuery::getGlobalStatus(snd_timer_gstatus_t *status)
{
    snd_timer_query_status(m_Info, status);
}

/*******************
 * TimerGlobalInfo *
 *******************/

TimerGlobalInfo::TimerGlobalInfo()
{
    snd_timer_ginfo_malloc(&m_Info);
}

TimerGlobalInfo::TimerGlobalInfo(const snd_timer_ginfo_t* other)
{
    snd_timer_ginfo_malloc(&m_Info);
    snd_timer_ginfo_copy(m_Info, other);
}

TimerGlobalInfo::TimerGlobalInfo(const TimerGlobalInfo& other)
{
    snd_timer_ginfo_malloc(&m_Info);
    snd_timer_ginfo_copy(m_Info, other.m_Info);
}

TimerGlobalInfo::~TimerGlobalInfo()
{
    snd_timer_ginfo_free(m_Info);
}

TimerGlobalInfo*
TimerGlobalInfo::clone()
{
    return new TimerGlobalInfo(m_Info);
}

TimerGlobalInfo&
TimerGlobalInfo::operator=(const TimerGlobalInfo& other)
{
    snd_timer_ginfo_copy(m_Info, other.m_Info);
    return *this;
}

void
TimerGlobalInfo::setTimerId(const TimerId& tid)
{
    m_Id = tid;
    snd_timer_ginfo_set_tid (m_Info, m_Id.m_Info);
}

TimerId&
TimerGlobalInfo::getTimerId()
{
    m_Id = TimerId(snd_timer_ginfo_get_tid (m_Info));
    return m_Id;
}

unsigned int
TimerGlobalInfo::getFlags()
{
    return snd_timer_ginfo_get_flags (m_Info);
}

int
TimerGlobalInfo::getCard()
{
    return snd_timer_ginfo_get_card (m_Info);
}

QString
TimerGlobalInfo::getId()
{
    return QString(snd_timer_ginfo_get_id (m_Info));
}

QString
TimerGlobalInfo::getName()
{
    return QString(snd_timer_ginfo_get_name (m_Info));
}

unsigned long
TimerGlobalInfo::getResolution()
{
    return snd_timer_ginfo_get_resolution (m_Info);
}

unsigned long
TimerGlobalInfo::getMinResolution()
{
    return snd_timer_ginfo_get_resolution_min (m_Info);
}

unsigned long
TimerGlobalInfo::getMaxResolution()
{
    return snd_timer_ginfo_get_resolution_max(m_Info);
}

unsigned int
TimerGlobalInfo::getClients()
{
    return snd_timer_ginfo_get_clients(m_Info);
}

int 
TimerGlobalInfo::getSizeOfInfo() const
{
    return snd_timer_ginfo_sizeof();
}

/***************
 * TimerParams *
 ***************/

TimerParams::TimerParams()
{
    snd_timer_params_malloc (&m_Info);
}

TimerParams::TimerParams(const snd_timer_params_t *other)
{
    snd_timer_params_malloc (&m_Info);
    snd_timer_params_copy (m_Info, other);
}

TimerParams::TimerParams(const TimerParams& other)
{
    snd_timer_params_malloc (&m_Info);
    snd_timer_params_copy (m_Info, other.m_Info);
}

TimerParams::~TimerParams()
{
    snd_timer_params_free (m_Info);
}

TimerParams*
TimerParams::clone()
{
    return new TimerParams(m_Info);
}

TimerParams&
TimerParams::operator=(const TimerParams& other)
{
    snd_timer_params_copy (m_Info, other.m_Info);
    return *this;
}

void
TimerParams::setAutoStart(bool auto_start)
{
    snd_timer_params_set_auto_start (m_Info, auto_start ? 1 : 0);
}

bool
TimerParams::getAutoStart()
{
    return (snd_timer_params_get_auto_start (m_Info) != 0);
}

void
TimerParams::setExclusive(bool exclusive)
{
    snd_timer_params_set_exclusive (m_Info, exclusive ? 1 : 0);
}

bool
TimerParams::getExclusive()
{
    return (snd_timer_params_get_exclusive (m_Info) != 0);
}

void
TimerParams::setEarlyEvent(bool early_event)
{
    snd_timer_params_set_early_event (m_Info, early_event ? 1 : 0);
}

bool
TimerParams::getEarlyEvent()
{
    return (snd_timer_params_get_early_event (m_Info) != 0);
}

void
TimerParams::setTicks(long ticks)
{
    snd_timer_params_set_ticks (m_Info, ticks);
}

long
TimerParams::getTicks()
{
    return snd_timer_params_get_ticks (m_Info);
}

void
TimerParams::setQueueSize(long queue_size)
{
    snd_timer_params_set_queue_size (m_Info, queue_size);
}

long
TimerParams::getQueueSize()
{
    return snd_timer_params_get_queue_size (m_Info);
}

void
TimerParams::setFilter(unsigned int filter)
{
    snd_timer_params_set_filter (m_Info, filter);
}

unsigned int
TimerParams::getFilter()
{
    return snd_timer_params_get_filter (m_Info);
}

int 
TimerParams::getSizeOfInfo() const
{
    return snd_timer_params_sizeof();
}

/***************
 * TimerStatus *
 ***************/

TimerStatus::TimerStatus()
{
    snd_timer_status_malloc (&m_Info);
}

TimerStatus::TimerStatus(const snd_timer_status_t *other)
{
    snd_timer_status_malloc (&m_Info);
    snd_timer_status_copy (m_Info, other);
}

TimerStatus::TimerStatus(const TimerStatus& other)
{
    snd_timer_status_malloc (&m_Info);
    snd_timer_status_copy (m_Info, other.m_Info);
}

TimerStatus::~TimerStatus()
{
    snd_timer_status_free (m_Info);
}

TimerStatus*
TimerStatus::clone()
{
    return new TimerStatus(m_Info);
}

TimerStatus&
TimerStatus::operator=(const TimerStatus& other)
{
    snd_timer_status_copy (m_Info, other.m_Info);
    return *this;
}

snd_htimestamp_t
TimerStatus::getTimestamp()
{
    return snd_timer_status_get_timestamp (m_Info);
}

long
TimerStatus::getResolution()
{
    return snd_timer_status_get_resolution (m_Info);
}

long
TimerStatus::getLost()
{
    return snd_timer_status_get_lost (m_Info);
}

long
TimerStatus::getOverrun()
{
    return snd_timer_status_get_overrun (m_Info);
}

long
TimerStatus::getQueue()
{
    return snd_timer_status_get_queue (m_Info);
}

int 
TimerStatus::getSizeOfInfo() const
{
    return snd_timer_status_sizeof();
}

/*********
 * Timer *
 *********/

Timer::Timer( const QString& deviceName, int openMode, QObject* parent )
    : QObject(parent),
    m_asyncHandler(NULL),
    m_deviceName(deviceName)
{
    CHECK_ERROR( snd_timer_open( &m_Info, m_deviceName.toLocal8Bit().data(), 
                                 openMode ));
}

Timer::Timer( const QString& deviceName, int openMode, snd_config_t* conf, 
              QObject* parent )
    : QObject(parent),
    m_asyncHandler(NULL),
    m_deviceName(deviceName)
{
    CHECK_ERROR( snd_timer_open_lconf( &m_Info, 
                                       m_deviceName.toLocal8Bit().data(), 
                                       openMode, conf ));
}

Timer::Timer( TimerId& id, int openMode, QObject* parent )
    : QObject(parent), 
    m_asyncHandler(NULL)
{
    m_deviceName = QString("hw:CLASS=%1,SCLASS=%2,CARD=%3,DEV=%4,SUBDEV=%5")
    .arg(id.getClass())
    .arg(id.getSlaveClass())
    .arg(id.getCard())
    .arg(id.getDevice())
    .arg(id.getSubdevice());
    CHECK_ERROR( snd_timer_open( &m_Info, 
                                 m_deviceName.toLocal8Bit().data(), 
                                 openMode ));
}

Timer::Timer( int cls, int scls, int card, int dev, int sdev, 
              int openMode, QObject* parent )
    : QObject(parent), 
    m_asyncHandler(NULL)
{
    m_deviceName = QString("hw:CLASS=%1,SCLASS=%2,CARD=%3,DEV=%4,SUBDEV=%5")
        .arg(cls)
        .arg(scls)
        .arg(card)
        .arg(dev)
        .arg(sdev);
    CHECK_ERROR( snd_timer_open( &m_Info, 
                                 m_deviceName.toLocal8Bit().data(), 
                                 openMode ));
}

Timer::~Timer()
{
    CHECK_WARNING(snd_timer_close(m_Info));
}

void
Timer::addAsyncTimerHandler(snd_async_callback_t callback, void *private_data)
{
    CHECK_WARNING(snd_async_add_timer_handler(&m_asyncHandler, m_Info, callback, private_data));
}

snd_timer_t* 
Timer::getTimerHandle()
{
    return snd_async_handler_get_timer(m_asyncHandler);
}

int
Timer::getPollDescriptorsCount()
{
    return snd_timer_poll_descriptors_count(m_Info);
}

void
Timer::pollDescriptors(struct pollfd *pfds, unsigned int space)
{
    CHECK_WARNING(snd_timer_poll_descriptors(m_Info, pfds, space));
}

void
Timer::pollDescriptorsRevents(struct pollfd *pfds, unsigned int nfds, unsigned short *revents)
{
    CHECK_WARNING(snd_timer_poll_descriptors_revents(m_Info, pfds, nfds, revents));
}

TimerInfo&
Timer::getTimerInfo()
{
    snd_timer_info (m_Info, m_TimerInfo.m_Info);
    return m_TimerInfo;
}

void
Timer::setTimerParams(const TimerParams& params)
{
    CHECK_WARNING( snd_timer_params(m_Info, params.m_Info) );
}

TimerStatus&
Timer::getTimerStatus()
{
    CHECK_WARNING( snd_timer_status(m_Info, m_TimerStatus.m_Info) );
    return m_TimerStatus;
}

void
Timer::start()
{
    CHECK_WARNING(snd_timer_start(m_Info));
}

void
Timer::stop()
{
    CHECK_WARNING(snd_timer_stop(m_Info));
}

void
Timer::continueRunning()
{
    CHECK_WARNING(snd_timer_continue(m_Info));
}

ssize_t
Timer::read(void *buffer, size_t size)
{
    return snd_timer_read(m_Info, buffer, size);
}

}
