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

#include "alsatimer.h"
#include <QReadLocker>
#include <QWriteLocker>
#include <cmath>
#include <cstdio>

/**
 * @file alsatimer.cpp
 * Implementation of classes managing ALSA Timers
 */

namespace drumstick {

/**
 * @addtogroup ALSATimer
 * @{
 *
 * Timers provide periodic time events to applications, and also to the ALSA
 * sequencer.
 *
 * There are two mechanisms to deliver the timer events. To use the callback
 * mechanism, a class must be derived from TimerEventHandler, and a instance
 * of the derived class must be assigned to the Timer instance using
 * Timer::setHandler(). If the handler is not assigned, then the Timer instance
 * will generate the signal Timer::timerExpired().
 *
 * Classes:
 *
 * TimerInfo: ALSA Timer information container.
 *
 * This class is used to hold properties about ALSA Timers.
 *
 * TimerId: ALSA Timer identifier container.
 *
 * This class provides an unique identifier for a Timer.
 *
 * TimerGlobalInfo: Global timer information container.
 *
 * This class provides global timer parameters.
 *
 * TimerQuery: ALSA Timer inquiry helper.
 *
 * This class provides a mechanism to enumerate the available system timers.
 *
 * TimerParams: ALSA Timer parameters container.
 *
 * This class provides several parameters about a Timer.
 *
 * TimerStatus: ALSA Timer status container.
 *
 * This class provides some status information about a Timer.
 *
 * TimerEventHandler: ALSA Timer events handler.
 *
 * This abstract class is used to define an interface that other class can
 * implement to receive timer events.
 *
 * Timer: ALSA Timer management.
 *
 * This class represents an ALSA timer object.
 *
 * @see http://www.alsa-project.org/alsa-doc/alsa-lib/group___timer.html
 * @}
 */

/**
 * Constructor
 */
TimerInfo::TimerInfo()
{
    snd_timer_info_malloc(&m_Info);
}

/**
 * Cosntructor
 * @param other ALSA timer info object pointer
 */
TimerInfo::TimerInfo(const snd_timer_info_t *other)
{
    snd_timer_info_malloc(&m_Info);
    snd_timer_info_copy(m_Info, other);
}

/**
 * Copy constructor
 * @param other Existing TimerInfo object reference
 */
TimerInfo::TimerInfo(const TimerInfo& other)
{
    snd_timer_info_malloc(&m_Info);
    snd_timer_info_copy(m_Info, other.m_Info);
}

/**
 * Destructor
 */
TimerInfo::~TimerInfo()
{
    snd_timer_info_free(m_Info);
}

/**
 * Copy the current object
 * @return Pointer to the new object
 */
TimerInfo*
TimerInfo::clone()
{
    return new TimerInfo(m_Info);
}

/**
 * Assignment operator
 * @param other Existing TimerInfo object reference
 */
TimerInfo&
TimerInfo::operator=(const TimerInfo& other)
{
    snd_timer_info_copy(m_Info, other.m_Info);
    return *this;
}

/**
 * Check if the timer is slave (depends on another device)
 * @return True if the timer is slave
 */
bool
TimerInfo::isSlave()
{
    return (snd_timer_info_is_slave(m_Info) != 0);
}

/**
 * Gets the card number
 * @return Card number
 */
int
TimerInfo::getCard()
{
    return snd_timer_info_get_card(m_Info);
}

/**
 * Gets the string identifier
 * @return String identifier
 */
QString
TimerInfo::getId()
{
    return QString(snd_timer_info_get_id(m_Info));
}

/**
 * Gets the timer name
 * @return Timer name
 */
QString
TimerInfo::getName()
{
    return QString(snd_timer_info_get_name(m_Info));
}

/**
 * Gets the timer resolution (timer period in nanoseconds)
 * @return Timer resolution in nanos
 */
long
TimerInfo::getResolution()
{
    return snd_timer_info_get_resolution(m_Info);
}

/**
 * Gets the timer frequency in Hz
 * @return Timer frequency in Hz
 */
long
TimerInfo::getFrequency()
{
    long res = getResolution();
    if (res > 0)
    {
        return 1000000000L / res;
    }
    return 0;
}

/**
 * Gets the size of the ALSA timer info object
 * @return Size of the ALSA object
 */
int
TimerInfo::getSizeOfInfo() const
{
    return snd_timer_info_sizeof();
}

/**
 * Gets the maximum timer ticks
 * @deprecated
 * @return Maximum timer ticks
 */
long
TimerInfo::getTicks()
{
    return snd_timer_info_get_ticks(m_Info);
}

/**
 * Constructor
 */
TimerId::TimerId()
{
    snd_timer_id_malloc(&m_Info);
}

/**
 * Constructor
 * @param other ALSA timer ID object pointer
 */
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

/**
 * Copy constructor
 * @param other Existing TimerId object reference
 */
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

/**
 * Constructor
 * @param cls  Class
 * @param scls Subclass
 * @param card Card
 * @param dev  Device
 * @param sdev Subdevice
 */
TimerId::TimerId(int cls, int scls, int card, int dev, int sdev)
{
    snd_timer_id_malloc(&m_Info);
    setClass(cls);
    setSlaveClass(scls);
    setCard(card);
    setDevice(dev);
    setSubdevice(sdev);
}

/**
 * Destructor
 */
TimerId::~TimerId()
{
    snd_timer_id_free(m_Info);
}

/**
 * Copy the object
 * @return Pointer to the new object
 */
TimerId*
TimerId::clone()
{
    return new TimerId(m_Info);
}

/**
 * Assignment operator
 * @param other Existing TimerId object reference
 * @return This object
 */
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

/**
 * Set the class identifier. Existing classes:
 * <ul>
 * <li> SND_TIMER_CLASS_SLAVE: slave timer</li>
 * <li> SND_TIMER_CLASS_GLOBAL: global timer</li>
 * <li> SND_TIMER_CLASS_CARD: card timer</li>
 * <li> SND_TIMER_CLASS_PCM: PCM timer</li>
 * </ul>
 * @param devclass Class identifier.
 */
void
TimerId::setClass(int devclass)
{
    snd_timer_id_set_class(m_Info, devclass);
}

/**
 * Gets the class identifier.
 * @return Class identifier
 * @see setClass()
 */
int
TimerId::getClass()
{
    return snd_timer_id_get_class(m_Info);
}

/**
 * Sets the Slave class
 * @param devsclass Slave class
 */
void
TimerId::setSlaveClass(int devsclass)
{
    snd_timer_id_set_sclass(m_Info, devsclass);
}

/**
 * Gets the slave class
 * @return Slave class
 */
int
TimerId::getSlaveClass()
{
    return snd_timer_id_get_sclass(m_Info);
}

/**
 * Sets the card number
 * @param card Card number
 */
void
TimerId::setCard(int card)
{
    snd_timer_id_set_card(m_Info, card);
}

/**
 * Gets the card number
 * @return Card number
 */
int
TimerId::getCard()
{
    return snd_timer_id_get_card(m_Info);
}

/**
 * Sets the device number
 * @param device Device number
 */
void
TimerId::setDevice(int device)
{
    snd_timer_id_set_device(m_Info, device);
}

/**
 * Gets the device number
 * @return Device number
 */
int
TimerId::getDevice()
{
    return snd_timer_id_get_device(m_Info);
}

/**
 * Sets the subdevice number
 * @param subdevice Subdevice number
 */
void
TimerId::setSubdevice(int subdevice)
{
    snd_timer_id_set_subdevice (m_Info, subdevice);
}

/**
 * Gets the subdevice number
 * @return Subdevice number
 */
int
TimerId::getSubdevice()
{
    return snd_timer_id_get_subdevice(m_Info);
}

/**
 * Gets the size of the ALSA timer ID object
 * @return Size of the ALSA object
 */
int
TimerId::getSizeOfInfo() const
{
    return snd_timer_id_sizeof();
}

/**
 * Constructor
 * @param deviceName Device name, usually "hw"
 * @param openMode Open mode (unknown values)
 */
TimerQuery::TimerQuery(const QString& deviceName, int openMode)
{
    CHECK_WARNING( snd_timer_query_open( &m_Info,
                                         deviceName.toLocal8Bit().data(),
                                         openMode ));
    readTimers();
}

/**
 * Constructor
 * @param deviceName Device name, usually "hw"
 * @param openMode Open mode (unknown values)
 * @param conf ALSA configuration object pointer
 */
TimerQuery::TimerQuery( const QString& deviceName, int openMode,
                        snd_config_t* conf )
{
    CHECK_WARNING( snd_timer_query_open_lconf( &m_Info,
                                               deviceName.toLocal8Bit().data(),
                                               openMode, conf ));
    readTimers();
}

/**
 * Destructor
 */
TimerQuery::~TimerQuery()
{
    freeTimers();
    snd_timer_query_close(m_Info);
}

/**
 * Enumerate the available timers storing the results into an internal list
 */
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

/**
 * Release the internal list of timers
 */
void
TimerQuery::freeTimers()
{
    m_timers.clear();
}

/**
 * Get a TimerGlobalInfo object
 * @return TimerGlobalInfo object reference
 */
TimerGlobalInfo&
TimerQuery::getGlobalInfo()
{
    snd_timer_query_info(m_Info, m_GlobalInfo.m_Info);
    return m_GlobalInfo;
}

/**
 * Sets the global parameters
 * @param params Pointer to an ALSA timer global parameters object
 */
void
TimerQuery::setGlobalParams(snd_timer_gparams_t* params)
{
    snd_timer_query_params(m_Info, params);
}

/**
 * Gets the global timer parameters
 * @param params Pointer to an ALSA timer global parameters object
 */
void
TimerQuery::getGlobalParams(snd_timer_gparams_t* params)
{
    snd_timer_query_params(m_Info, params);
}

/**
 * Gets the global timer status
 * @param status Pointer to an ALSA timer global status object
 */
void
TimerQuery::getGlobalStatus(snd_timer_gstatus_t *status)
{
    snd_timer_query_status(m_Info, status);
}

/**
 * Default constructor
 */
TimerGlobalInfo::TimerGlobalInfo()
{
    snd_timer_ginfo_malloc(&m_Info);
}

/**
 * Constructor
 * @param other ALSA global info object pointer
 */
TimerGlobalInfo::TimerGlobalInfo(const snd_timer_ginfo_t* other)
{
    snd_timer_ginfo_malloc(&m_Info);
    snd_timer_ginfo_copy(m_Info, other);
}

/**
 * Copy constructor
 * @param other Existing TimerGlobalInfo object reference
 */
TimerGlobalInfo::TimerGlobalInfo(const TimerGlobalInfo& other)
{
    snd_timer_ginfo_malloc(&m_Info);
    snd_timer_ginfo_copy(m_Info, other.m_Info);
}

/**
 * Destructor
 */
TimerGlobalInfo::~TimerGlobalInfo()
{
    snd_timer_ginfo_free(m_Info);
}

/**
 * Copy the current object
 * @return Pointer to the new object
 */
TimerGlobalInfo*
TimerGlobalInfo::clone()
{
    return new TimerGlobalInfo(m_Info);
}

/**
 * Assignment operator
 * @param other Existing TimerGlobalInfo object reference
 * @return This object
 */
TimerGlobalInfo&
TimerGlobalInfo::operator=(const TimerGlobalInfo& other)
{
    snd_timer_ginfo_copy(m_Info, other.m_Info);
    return *this;
}

/**
 * Sets the timer identifier
 * @param tid TimerId object reference
 */
void
TimerGlobalInfo::setTimerId(const TimerId& tid)
{
    m_Id = tid;
    snd_timer_ginfo_set_tid (m_Info, m_Id.m_Info);
}

/**
 * Gets the timer identifier
 * @return TimerId object reference
 */
TimerId&
TimerGlobalInfo::getTimerId()
{
    m_Id = TimerId(snd_timer_ginfo_get_tid (m_Info));
    return m_Id;
}

/**
 * Gets the flags
 * @return Undocumented flags
 */
unsigned int
TimerGlobalInfo::getFlags()
{
    return snd_timer_ginfo_get_flags (m_Info);
}

/**
 * Gets the card number
 * @return Card number
 */
int
TimerGlobalInfo::getCard()
{
    return snd_timer_ginfo_get_card (m_Info);
}

/**
 * Gets the timer ID string
 * @return Timer ID string
 */
QString
TimerGlobalInfo::getId()
{
    return QString(snd_timer_ginfo_get_id (m_Info));
}

/**
 * Gets the timer name
 * @return Timer name
 */
QString
TimerGlobalInfo::getName()
{
    return QString(snd_timer_ginfo_get_name (m_Info));
}

/**
 * Gets the timer resolution in ns
 * @return Timer resolution in ns
 */
unsigned long
TimerGlobalInfo::getResolution()
{
    return snd_timer_ginfo_get_resolution (m_Info);
}

/**
 * Gets timer minimal resolution in ns
 * @return Minimal resolution in ns
 */
unsigned long
TimerGlobalInfo::getMinResolution()
{
    return snd_timer_ginfo_get_resolution_min (m_Info);
}

/**
 * Gets timer maximal resolution in ns
 * @return Maximal resolution in ns
 */
unsigned long
TimerGlobalInfo::getMaxResolution()
{
    return snd_timer_ginfo_get_resolution_max(m_Info);
}

/**
 * Gets current timer clients
 * @return Current clients
 */
unsigned int
TimerGlobalInfo::getClients()
{
    return snd_timer_ginfo_get_clients(m_Info);
}

/**
 * Gets the size of the ALSA timer global info object
 * @return Size of the ALSA object
 */
int
TimerGlobalInfo::getSizeOfInfo() const
{
    return snd_timer_ginfo_sizeof();
}

/**
 * Default constructor
 */
TimerParams::TimerParams()
{
    snd_timer_params_malloc (&m_Info);
}

/**
 * Constructor
 * @param other Pointer to an ALSA timer parameters object
 */
TimerParams::TimerParams(const snd_timer_params_t *other)
{
    snd_timer_params_malloc (&m_Info);
    snd_timer_params_copy (m_Info, other);
}

/**
 * Copy constructor
 * @param other Existing TimerParams object reference
 */
TimerParams::TimerParams(const TimerParams& other)
{
    snd_timer_params_malloc (&m_Info);
    snd_timer_params_copy (m_Info, other.m_Info);
}

/**
 * Destructor
 * @return
 */
TimerParams::~TimerParams()
{
    snd_timer_params_free (m_Info);
}

/**
 * Copy the current object
 * @return Pointer to the new object
 */
TimerParams*
TimerParams::clone()
{
    return new TimerParams(m_Info);
}

/**
 * Assignment operator
 * @param other Existing TimerParams object reference
 * @return This object
 */
TimerParams&
TimerParams::operator=(const TimerParams& other)
{
    snd_timer_params_copy (m_Info, other.m_Info);
    return *this;
}

/**
 * Sets the automatic start flag
 * @param auto_start Value for the automatic start flag
 */
void
TimerParams::setAutoStart(bool auto_start)
{
    snd_timer_params_set_auto_start (m_Info, auto_start ? 1 : 0);
}

/**
 * Gets the automatic start flag
 * @return True if the timer starts automatically
 */
bool
TimerParams::getAutoStart()
{
    return (snd_timer_params_get_auto_start (m_Info) != 0);
}

/**
 * Sets the exclusive flag
 * @param exclusive True if the timer has the exclusive flag
 */
void
TimerParams::setExclusive(bool exclusive)
{
    snd_timer_params_set_exclusive (m_Info, exclusive ? 1 : 0);
}

/**
 * Gets the timer's exclusive flag
 * @return True if the timer has the exclusive flag
 */
bool
TimerParams::getExclusive()
{
    return (snd_timer_params_get_exclusive (m_Info) != 0);
}

/**
 * Sets the timer early event
 * @param early_event Timer early event
 */
void
TimerParams::setEarlyEvent(bool early_event)
{
    snd_timer_params_set_early_event (m_Info, early_event ? 1 : 0);
}

/**
 * Gets the timer early event
 * @return Timer early event
 */
bool
TimerParams::getEarlyEvent()
{
    return (snd_timer_params_get_early_event (m_Info) != 0);
}

/**
 * Sets the timer ticks
 * @param ticks Timer ticks
 */
void
TimerParams::setTicks(long ticks)
{
    snd_timer_params_set_ticks (m_Info, ticks);
}

/**
 * Gets the timer ticks
 * @return Timer ticks
 */
long
TimerParams::getTicks()
{
    return snd_timer_params_get_ticks (m_Info);
}

/**
 * Sets the queue size (32-1024)
 * @param queue_size Queue size
 */
void
TimerParams::setQueueSize(long queue_size)
{
    snd_timer_params_set_queue_size (m_Info, queue_size);
}

/**
 * Gets the queue size
 * @return Queue size
 */
long
TimerParams::getQueueSize()
{
    return snd_timer_params_get_queue_size (m_Info);
}

/**
 * Sets the event filter
 * @param filter Event filter
 */
void
TimerParams::setFilter(unsigned int filter)
{
    snd_timer_params_set_filter (m_Info, filter);
}

/**
 * Gets the event filter
 * @return Event filter
 */
unsigned int
TimerParams::getFilter()
{
    return snd_timer_params_get_filter (m_Info);
}

/**
 * Gets the size of the ALSA timer parameters object
 * @return Size of the ALSA object
 */
int
TimerParams::getSizeOfInfo() const
{
    return snd_timer_params_sizeof();
}

/**
 * Default constructor
 */
TimerStatus::TimerStatus()
{
    snd_timer_status_malloc (&m_Info);
}

/**
 * Constructor
 * @param other Pointer to an existing ALSA timer status object
 */
TimerStatus::TimerStatus(const snd_timer_status_t *other)
{
    snd_timer_status_malloc (&m_Info);
    snd_timer_status_copy (m_Info, other);
}

/**
 * Copy constructor
 * @param other Existing TimerStatus object reference
 */
TimerStatus::TimerStatus(const TimerStatus& other)
{
    snd_timer_status_malloc (&m_Info);
    snd_timer_status_copy (m_Info, other.m_Info);
}

/**
 * Destructor
 */
TimerStatus::~TimerStatus()
{
    snd_timer_status_free (m_Info);
}

/**
 * Copy the current object
 * @return Pointer to the new object
 */
TimerStatus*
TimerStatus::clone()
{
    return new TimerStatus(m_Info);
}

/**
 * Assignment operator
 * @param other Existing TimerStatus object reference
 * @return This object
 */
TimerStatus&
TimerStatus::operator=(const TimerStatus& other)
{
    snd_timer_status_copy (m_Info, other.m_Info);
    return *this;
}

/**
 * Gets the high resolution time-stamp
 * @return High resolution time-stamp
 */
snd_htimestamp_t
TimerStatus::getTimestamp()
{
    return snd_timer_status_get_timestamp (m_Info);
}

/**
 * Gets the resolution in us
 * @return Resolution in us
 */
long
TimerStatus::getResolution()
{
    return snd_timer_status_get_resolution (m_Info);
}

/**
 * Gets the master tick lost count
 * @return Master tick lost count
 */
long
TimerStatus::getLost()
{
    return snd_timer_status_get_lost (m_Info);
}

/**
 * Gets the overrun count
 * @return Overrun count
 */
long
TimerStatus::getOverrun()
{
    return snd_timer_status_get_overrun (m_Info);
}

/**
 * Gets the count of used queue elements
 * @return Count of used queue elements
 */
long
TimerStatus::getQueue()
{
    return snd_timer_status_get_queue (m_Info);
}

/**
 * Gets the size of the ALSA timer status object
 * @return Size of the ALSA object
 */
int
TimerStatus::getSizeOfInfo() const
{
    return snd_timer_status_sizeof();
}

/**
 * Constructor.
 * Open flags can be a combination of the following constants:
 * <ul>
 * <li>SND_TIMER_OPEN_NONBLOCK: non-blocking behavior</li>
 * <li>SND_TIMER_OPEN_TREAD: enhanced read, use time-stamps and event notification</li>
 * </ul>
 * @param deviceName Name of the device
 * @param openMode Open mode flags bitmap
 * @param parent Optional parent object
 */
Timer::Timer( const QString& deviceName, int openMode, QObject* parent )
    : QObject(parent),
    m_asyncHandler(NULL),
    m_handler(NULL),
    m_thread(NULL),
    m_deviceName(deviceName)
{
    CHECK_ERROR( snd_timer_open( &m_Info, m_deviceName.toLocal8Bit().data(),
                                 openMode ));
}

/**
 * Constructor.
 * Open flags can be a combination of the following constants:
 * <ul>
 * <li>SND_TIMER_OPEN_NONBLOCK: non-blocking behavior</li>
 * <li>SND_TIMER_OPEN_TREAD: enhanced read, use time-stamps and event notification</li>
 * </ul>
 * @param deviceName Name of the device
 * @param openMode Open mode flags bitmap
 * @param conf ALSA configuration object pointer
 * @param parent Optional parent object
 */
Timer::Timer( const QString& deviceName, int openMode, snd_config_t* conf,
              QObject* parent )
    : QObject(parent),
    m_asyncHandler(NULL),
    m_handler(NULL),
    m_thread(NULL),
    m_deviceName(deviceName)
{
    CHECK_ERROR( snd_timer_open_lconf( &m_Info,
                                       m_deviceName.toLocal8Bit().data(),
                                       openMode, conf ));
}

/**
 * Constructor
 * Open flags can be a combination of the following constants:
 * <ul>
 * <li>SND_TIMER_OPEN_NONBLOCK: non-blocking behavior</li>
 * <li>SND_TIMER_OPEN_TREAD: enhanced read, use time-stamps and event notification</li>
 * </ul>
 * @param id TimerId object reference
 * @param openMode Open mode flags bitmap
 * @param parent Optional parent object
 */
Timer::Timer( TimerId& id, int openMode, QObject* parent )
    : QObject(parent),
    m_asyncHandler(NULL),
    m_handler(NULL),
    m_thread(NULL)
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

/**
 * Constructor.
 * Open flags can be a combination of the following constants:
 * <ul>
 * <li>SND_TIMER_OPEN_NONBLOCK: non-blocking behavior</li>
 * <li>SND_TIMER_OPEN_TREAD: enhanced read, use time-stamps and event notification</li>
 * </ul>
 * @param cls Class
 * @param scls Subclass
 * @param card Card
 * @param dev Device
 * @param sdev Subdevice
 * @param openMode Open mode flags bitmap
 * @param parent Optional parent object
 */
Timer::Timer( int cls, int scls, int card, int dev, int sdev,
              int openMode, QObject* parent )
    : QObject(parent),
    m_asyncHandler(NULL),
    m_handler(NULL),
    m_thread(NULL)
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

/**
 * Destructor.
 */
Timer::~Timer()
{
    stopEvents();
    if (m_thread != NULL)
        delete m_thread;
    CHECK_WARNING(snd_timer_close(m_Info));
}

/**
 * Adds an asynchronous timer handler function.
 * @param callback Function handler
 * @param private_data Any data that will be passed to the callback
 */
void
Timer::addAsyncTimerHandler(snd_async_callback_t callback, void *private_data)
{
    CHECK_WARNING(snd_async_add_timer_handler(&m_asyncHandler, m_Info, callback, private_data));
}

/**
 * Gets the ALSA timer handle
 * @return ALSA timer handle
 */
snd_timer_t*
Timer::getTimerHandle()
{
    return snd_async_handler_get_timer(m_asyncHandler);
}

/**
 * Gets the count of poll descriptors
 * @return Count of poll descriptors
 */
int
Timer::getPollDescriptorsCount()
{
    return snd_timer_poll_descriptors_count(m_Info);
}

/**
 * Gets poll descriptors
 * @param pfds  Pointer to a pollfd array
 * @param space Number of pollfd elements available
 */
void
Timer::pollDescriptors(struct pollfd *pfds, unsigned int space)
{
    CHECK_WARNING(snd_timer_poll_descriptors(m_Info, pfds, space));
}

/**
 * Gets returned events from poll descriptors
 * @param pfds Pointer to a pollfd array
 * @param nfds Number of pollfd elements available
 * @param revents Returned events
 */
void
Timer::pollDescriptorsRevents(struct pollfd *pfds, unsigned int nfds, unsigned short *revents)
{
    CHECK_WARNING(snd_timer_poll_descriptors_revents(m_Info, pfds, nfds, revents));
}

/**
 * Gets the timer info object
 * @return TimerInfo object reference
 */
TimerInfo&
Timer::getTimerInfo()
{
    snd_timer_info (m_Info, m_TimerInfo.m_Info);
    return m_TimerInfo;
}

/**
 * Sets the timer parameters
 * @param params TimerParams object reference
 */
void
Timer::setTimerParams(const TimerParams& params)
{
    CHECK_WARNING( snd_timer_params(m_Info, params.m_Info) );
}

/**
 * Gets the timer status
 * @return TimerStatus object reference
 */
TimerStatus&
Timer::getTimerStatus()
{
    CHECK_WARNING( snd_timer_status(m_Info, m_TimerStatus.m_Info) );
    return m_TimerStatus;
}

/**
 * Start rolling the timer
 */
void
Timer::start()
{
    CHECK_WARNING(snd_timer_start(m_Info));
}

/**
 * Stop rolling the timer
 */
void
Timer::stop()
{
    CHECK_WARNING(snd_timer_stop(m_Info));
}

/**
 * Continue rolling the timer
 */
void
Timer::continueRunning()
{
    CHECK_WARNING(snd_timer_continue(m_Info));
}

/**
 * Read bytes from the timer handle
 * @param buffer Buffer to store the input bytes
 * @param size  Input buffer size in bytes
 * @return Bytes read from the timer
 */
ssize_t
Timer::read(void *buffer, size_t size)
{
    return snd_timer_read(m_Info, buffer, size);
}

/**
 * Internal function to deliver the timer events using one of the two available
 * methods:
 * <ul>
 * <li>TimerEventHandler instance pointer provided in Timer::setHandler()</li>
 * <li>A signal Timer::timerExpired() is emitted, otherwise</li>
 * </ul>
 */
void
Timer::doEvents()
{
    snd_timer_tread_t tr;
    while ( read(&tr, sizeof(tr)) == sizeof(tr) ) {
        int msecs = ((tr.tstamp.tv_sec - m_last_time.tv_sec) * 1000) +
                    round((tr.tstamp.tv_nsec - m_last_time.tv_nsec) / 1000000.0);
        m_last_time = tr.tstamp;
        if ( m_handler != NULL )
            m_handler->handleTimerEvent(tr.val, msecs);
        else
            emit timerExpired(tr.val, msecs);
    }
}

/**
 * Starts the events dispatching thread
 */
void Timer::startEvents()
{
    m_last_time = getTimerStatus().getTimestamp();
    if (m_thread == NULL) {
        m_thread = new TimerInputThread(this, 500);
        m_thread->start();
    }
}

/**
 * Stops the events dispatching thread
 */
void Timer::stopEvents()
{
    int counter = 0;
    if (m_thread != NULL) {
        m_thread->stop();
        while (!m_thread->wait(500) && (counter < 10)) {
            counter++;
        }
        if (!m_thread->isFinished()) {
            m_thread->terminate();
        }
        delete m_thread;
    }
}

/**
 * Check and return the best available global TimerId in the system, meaning
 * the timer with higher frequency (or lesser period, resolution).
 * @return A TimerId object
 */
TimerId
Timer::bestGlobalTimerId()
{
    TimerId id;
    snd_timer_t* timer;
    snd_timer_info_t* info;
    long res, best_res = LONG_MAX;
    char timername[64];
    int test_devs[] = {
          SND_TIMER_GLOBAL_SYSTEM
        , SND_TIMER_GLOBAL_RTC
#ifdef SND_TIMER_GLOBAL_HPET
        , SND_TIMER_GLOBAL_HPET
#endif
#ifdef SND_TIMER_GLOBAL_HRTIMER
        , SND_TIMER_GLOBAL_HRTIMER
#endif
    };
    int max_global_timers = sizeof(test_devs)/sizeof(int);
    int clas = SND_TIMER_CLASS_GLOBAL;
    int scls = SND_TIMER_SCLASS_NONE;
    int card = 0;
    int dev  = SND_TIMER_GLOBAL_SYSTEM;
    int sdev = 0;
    int err = 0;
    int is_slave = 0;
    int i;
    snd_timer_info_alloca(&info);
    // default system timer
    id.setClass(clas);
    id.setSlaveClass(scls);
    id.setCard(card);
    id.setDevice(dev);
    id.setSubdevice(sdev);
    // select a non slave timer with the lowest resolution value
    for( i = 0; i < max_global_timers; ++i )
    {
        dev = test_devs[i];
        sprintf( timername, "hw:CLASS=%i,SCLASS=%i,CARD=%i,DEV=%i,SUBDEV=%i",
                 clas, scls, card, dev, sdev );
        err = snd_timer_open(&timer, timername, SND_TIMER_OPEN_NONBLOCK);
        if (err < 0) continue;
        err = snd_timer_info(timer, info);
        if (err == 0) {
            is_slave = snd_timer_info_is_slave(info);
            res = snd_timer_info_get_resolution(info);
            if ((is_slave == 0) && (best_res > res)) {
                best_res = res;
                id.setDevice(dev);
            }
        }
        snd_timer_close(timer);
    }
    return id;
}

/**
 * Check and return the best available global Timer in the system, meaning
 * the timer with higher frequency (or lesser period, resolution).
 * @param openMode Open mode flags
 * @param parent Optional parent object
 * @return A new Timer instance pointer
 */
Timer*
Timer::bestGlobalTimer(int openMode, QObject* parent)
{
    TimerId id = bestGlobalTimerId();
    return new Timer(id, openMode, parent);
}

/**
 * Loop reading and dispatching timer events.
 */
void
Timer::TimerInputThread::run()
{
    int err, count;
    struct pollfd *fds;
    if (m_timer == NULL) return;

    count = m_timer->getPollDescriptorsCount();
    fds = (pollfd *) calloc(count, sizeof(struct pollfd));
    if (fds == NULL) {
        qWarning() << "allocation error!";
        return;
    }
    fds->events = POLLIN;
    fds->revents = 0;

    try {
        while (!stopped() && (m_timer != NULL)) {
            m_timer->pollDescriptors(fds, count);
            if ((err = poll(fds, count, m_Wait)) < 0) {
                qWarning() << "poll error " << err << "(" << strerror(err) << ")";
                return;
            }
            if (err == 0) {
                qWarning() << "timer time out";
                return;
            }
            m_timer->doEvents();
        }
    } catch (...) {
        qWarning() << "exception in input thread";
    }
    free(fds);
}

/**
 * Returns the rolling state of the timer thread
 * @return The stopped state
 */
bool
Timer::TimerInputThread::stopped()
{
	QReadLocker locker(&m_mutex);
    return m_Stopped;
}

/**
 * Stop the thread
 */
void
Timer::TimerInputThread::stop()
{
	QWriteLocker locker(&m_mutex);
    m_Stopped = true;
}

} /* namespace drumstick */
