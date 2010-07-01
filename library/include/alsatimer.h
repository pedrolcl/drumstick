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

#ifndef DRUMSTICK_ALSATIMER_H
#define DRUMSTICK_ALSATIMER_H

#include "drumstickcommon.h"
#include <QList>
#include <QThread>
#include <QReadWriteLock>
#include <QPointer>

/**
 * @file alsatimer.h
 * Classes managing ALSA Timers
 * @defgroup ALSATimer ALSA Timers
 * @{
 */

namespace drumstick {

class TimerQuery;
class TimerId;
class TimerGlobalInfo;

/**
 * ALSA Timer information container.
 *
 * This class is used to hold properties about ALSA Timers.
 */
class DRUMSTICK_EXPORT TimerInfo
{
    friend class Timer;

public:
    TimerInfo();
    TimerInfo(const TimerInfo& other);
    TimerInfo(const snd_timer_info_t* other);
    virtual ~TimerInfo();
    TimerInfo* clone();
    TimerInfo& operator=(const TimerInfo& other);
    int getSizeOfInfo() const;
    
    bool isSlave();
    int getCard();
    QString getId();
    QString getName();
    long getResolution();
    long getFrequency();

protected:
    long getTicks() __attribute__((deprecated));
    
private:
    snd_timer_info_t *m_Info;
};

/**
 * ALSA Timer identifier container.
 *
 * This class provides an unique identifier for a Timer.
 */
class DRUMSTICK_EXPORT TimerId
{
    friend class TimerQuery;
    friend class TimerGlobalInfo;
    friend class QueueTimer;

public:
    TimerId();
    TimerId(const TimerId& other);
    TimerId(const snd_timer_id_t *other);
    TimerId(int cls, int scls, int card, int dev, int sdev);
    virtual ~TimerId();
    TimerId* clone();
    TimerId& operator=(const TimerId& other);
    int getSizeOfInfo() const;

    void setClass(int devclass);
    int getClass();
    void setSlaveClass(int devsclass);
    int getSlaveClass();
    void setCard(int card);
    int getCard();
    void setDevice(int device);
    int getDevice();
    void setSubdevice(int subdevice);
    int getSubdevice();

private:
    snd_timer_id_t *m_Info;
};

/**
 * List of timer identifiers
 */
typedef QList<TimerId> TimerIdList;

/**
 * Global timer information container.
 *
 * This class provides global timer parameters.
 */
class DRUMSTICK_EXPORT TimerGlobalInfo
{
    friend class TimerQuery;

public:
    TimerGlobalInfo();
    TimerGlobalInfo(const TimerGlobalInfo& other);
    TimerGlobalInfo(const snd_timer_ginfo_t* other);
    virtual ~TimerGlobalInfo();
    TimerGlobalInfo* clone();
    TimerGlobalInfo& operator=(const TimerGlobalInfo& other);
    int getSizeOfInfo() const;

    void setTimerId(const TimerId& tid);
    TimerId& getTimerId();
    unsigned int getFlags();
    int getCard();
    QString getId();
    QString getName();
    unsigned long getResolution();
    unsigned long getMinResolution();
    unsigned long getMaxResolution();
    unsigned int getClients();

private:
    snd_timer_ginfo_t* m_Info;
    TimerId m_Id;
};

/**
 * ALSA Timer inquiry helper.
 *
 * This class provides a mechanism to enumerate the available system timers.
 */
class DRUMSTICK_EXPORT TimerQuery
{
public:
    TimerQuery(const QString& deviceName, int openMode);
    TimerQuery(const QString& deviceName, int openMode, snd_config_t* conf);
    virtual ~TimerQuery();
    /**
     * Gets the list of available timers
     * @return List of TimerId objects
     */
    TimerIdList getTimers() const { return m_timers; }
    TimerGlobalInfo& getGlobalInfo();
    void setGlobalParams(snd_timer_gparams_t* params);
    void getGlobalParams(snd_timer_gparams_t* params);
    void getGlobalStatus(snd_timer_gstatus_t* status);

protected:
    void readTimers();
    void freeTimers();

private:
    snd_timer_query_t *m_Info;
    TimerIdList m_timers;
    TimerGlobalInfo m_GlobalInfo;
};

/**
 * ALSA Timer parameters container.
 *
 * This class provides several parameters about a Timer.
 */
class DRUMSTICK_EXPORT TimerParams
{
    friend class Timer;

public:
    TimerParams();
    TimerParams(const TimerParams& other);
    TimerParams(const snd_timer_params_t* other);
    virtual ~TimerParams();
    TimerParams* clone();
    TimerParams& operator=(const TimerParams& other);
    int getSizeOfInfo() const;

    void setAutoStart(bool auto_start);
    bool getAutoStart();
    void setExclusive(bool exclusive);
    bool getExclusive();
    void setEarlyEvent(bool early_event);
    bool getEarlyEvent();
    void setTicks(long ticks);
    long getTicks();
    void setQueueSize(long queue_size);
    long getQueueSize();
    void setFilter(unsigned int filter);
    unsigned int getFilter();

private:
    snd_timer_params_t* m_Info;
};

/**
 * ALSA Timer status container.
 *
 * This class provides some status information about a Timer.
 */
class DRUMSTICK_EXPORT TimerStatus
{
    friend class Timer;

public:
    TimerStatus();
    TimerStatus(const TimerStatus& other);
    TimerStatus(const snd_timer_status_t* other);
    virtual ~TimerStatus();
    TimerStatus* clone();
    TimerStatus& operator=(const TimerStatus& other);
    int getSizeOfInfo() const;

    snd_htimestamp_t getTimestamp();
    long getResolution();
    long getLost();
    long getOverrun();
    long getQueue();

private:
    snd_timer_status_t* m_Info;
};

/**
 * ALSA Timer events handler.
 *
 * This abstract class is used to define an interface that other class can
 * implement to receive timer events.
 */
class DRUMSTICK_EXPORT TimerEventHandler
{
public:
    /** Destructor */
    virtual ~TimerEventHandler() {}
    /**
     * Timer event handler. This method is called when the timer expires.
     * @param ticks The time in ticks.
     * @param msecs The time in milliseconds.
     */
    virtual void handleTimerEvent(int ticks, int msecs) = 0;
};

/**
 * ALSA Timer management.
 *
 * This class represents an ALSA timer object.
 */
class DRUMSTICK_EXPORT Timer : public QObject
{
    Q_OBJECT
    
private:
    /**
     * This class manages timer events input from ALSA
     */
    class TimerInputThread : public QThread
    {
    public: 
       /** Constructor */
       TimerInputThread(Timer* t, int timeout)
           : QThread(),
           m_timer(t),
           m_Wait(timeout),
           m_Stopped(false) {}
       /** Destructor */
       virtual ~TimerInputThread() {}
       virtual void run();
       bool stopped();
       void stop();       
    private:
        Timer* m_timer;
        int m_Wait;
        bool m_Stopped;
        QReadWriteLock m_mutex;        
    };

public:
    Timer(int cls, int scls, int card, int dev, int sdev, int openMode, QObject* parent = 0);
    Timer(const QString& deviceName, int openMode, QObject* parent = 0);
    Timer(const QString& deviceName, int openMode, snd_config_t* config, QObject* parent = 0);
    Timer(TimerId& id, int openMode, QObject* parent = 0);
    virtual ~Timer();
    
    static TimerId bestGlobalTimerId();
    static Timer* bestGlobalTimer(int openMode, QObject* parent = 0);
    /**
     * Gets the ALSA timer object.
     * @return ALSA timer object pointer.
     */
    snd_timer_t* getHandle() { return m_Info; }
    TimerInfo& getTimerInfo();
    TimerStatus& getTimerStatus();
    void setTimerParams(const TimerParams& params);
    
    void start();
    void stop();
    void continueRunning();

    void addAsyncTimerHandler(snd_async_callback_t callback, void *private_data);
    int getPollDescriptorsCount();
    void pollDescriptors(struct pollfd *pfds, unsigned int space);
    void pollDescriptorsRevents(struct pollfd *pfds, unsigned int nfds, unsigned short *revents);
    ssize_t read(void *buffer, size_t size);
    snd_timer_t* getTimerHandle();
    /**
     * Sets an event handler providing a method to be called when a timer expires.
     * @param h A TimerEventHandler instance.
     */
    void setHandler(TimerEventHandler* h) { m_handler = h; }
    void startEvents();
    void stopEvents();
    
protected:
    void doEvents();

signals:    
    /**
     * This signal is emitted when the timer has expired, if there is not an
     * event hander installed.
     *
     * @param ticks The time in ticks.
     * @param msecs The time in milliseconds.
     */
    void timerExpired(int ticks, int msecs);

private:
    snd_timer_t *m_Info;
    snd_async_handler_t *m_asyncHandler;
    TimerEventHandler* m_handler;
    QPointer<TimerInputThread> m_thread;
    TimerInfo m_TimerInfo;
    TimerStatus m_TimerStatus;
    QString m_deviceName;
    snd_htimestamp_t m_last_time;
};

} /* namespace drumstick */

/** @} */

#endif /* DRUMSTICK_ALSATIMER_H */
