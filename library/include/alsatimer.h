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

#ifndef INCLUDED_TIMER_H
#define INCLUDED_TIMER_H

#include "commons.h"
#include <QList>

namespace ALSA
{

class TimerQuery;
class TimerId;
class TimerGlobalInfo;

class TimerInfo
{
public:
	TimerInfo();
	TimerInfo(const snd_timer_info_t* other);
	virtual ~TimerInfo();
	TimerInfo* clone();

	bool isSlave();
	int getCard();
	QString getId();
	QString getName();
	long getResolution();
	long getFrequency();
	
private:
	snd_timer_info_t *m_Info;	
};

class TimerId
{
	friend class TimerQuery;
	friend class TimerGlobalInfo;
	
public:
	TimerId();
	TimerId(const TimerId& other);
	TimerId(const snd_timer_id_t *other);
	virtual ~TimerId();
	TimerId* clone();
	TimerId& operator=(const TimerId& other);
	
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

typedef QList<TimerId> TimerIdList;

class TimerGlobalInfo
{
public:	
	TimerGlobalInfo();
	TimerGlobalInfo(snd_timer_ginfo_t* other);
	virtual ~TimerGlobalInfo();
	TimerGlobalInfo* clone();
	
	void setTimerId(TimerId *tid);
	TimerId* getTimerId();
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
};

class TimerQuery
{
public:
	TimerQuery(const QString& deviceName, int openMode);
	virtual ~TimerQuery();
	
	TimerIdList getTimers() { return m_timers; }
	TimerGlobalInfo* getGlobalInfo();
	void setGlobalParams(snd_timer_gparams_t* params);
	void getGlobalParams(snd_timer_gparams_t* params);
	void getGlobalStatus(snd_timer_gstatus_t* status);
	
protected:
	void readTimers();
	void freeTimers();
	
private:
	snd_timer_query_t *m_Info;
	TimerIdList m_timers;
};

class TimerParams
{
	friend class Timer;
	
public:
	TimerParams();
	TimerParams(const snd_timer_params_t* other);
	virtual ~TimerParams();
	TimerParams* clone();
	
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

class TimerStatus
{
public:
	TimerStatus();
	TimerStatus(snd_timer_status_t* other);
	virtual ~TimerStatus();
	TimerStatus* clone();
	
	snd_htimestamp_t getTimestamp();
	long getResolution();
	long getLost();
	long getOverrun();
	long getQueue();
	
private:
	snd_timer_status_t* m_Info;
};

class Timer
{
public:
	Timer(const QString& deviceName, int openMode);
	Timer(TimerId* id, int openMode);
	virtual ~Timer();
	snd_timer_t* getHandle() { return m_Info; }
	
	void addAsyncTimerHandler(snd_async_callback_t callback, void *private_data);
	int getPollDescriptorsCount();
	void pollDescriptors(struct pollfd *pfds, unsigned int space);
	void pollDescriptorsRevents(struct pollfd *pfds, unsigned int nfds, unsigned short *revents);
	TimerInfo* getTimerInfo();
	void setTimerParams(TimerParams* params);
	TimerStatus* getTimerStatus();
	void start();
	void stop();
	void continueRunning();
	ssize_t read(void *buffer, size_t size);

private:
	snd_timer_t *m_Info;
	snd_async_handler_t *m_asyncHandler;
};

}

#endif /*INCLUDED_TIMER_H*/
