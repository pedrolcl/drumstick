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

 
#ifndef INCLUDED_QUEUE_H
#define INCLUDED_QUEUE_H

#include "commons.h"

namespace ALSA 
{
namespace Sequencer 
{
 
class MidiClient;

class QueueInfo
{
	friend class MidiQueue;
	
public:
    QueueInfo();
    QueueInfo(snd_seq_queue_info_t* other);
    virtual ~QueueInfo();
    QueueInfo* clone();
    
    int getId();
    QString getName();
    int getOwner();
    bool getLocked();
    unsigned int getFlags();
    void setName(QString value);
    void setOwner(int value);
    void setFlags(unsigned int value);
  
private:
    snd_seq_queue_info_t* m_Info;
};
 
  
class QueueStatus
{
	friend class MidiQueue;
	
public:
    QueueStatus();
    QueueStatus(snd_seq_queue_status_t* other);
    virtual ~QueueStatus();
    QueueStatus* clone();
    
    int getId();
    int getEvents();
    const snd_seq_real_time_t* getRealtime();
    unsigned int getStatusBits();
    snd_seq_tick_time_t getTickTime();
  
private:
    snd_seq_queue_status_t* m_Info;
};
 
  
class QueueTempo
{
	friend class MidiQueue;
	
public:
    QueueTempo();
    QueueTempo(snd_seq_queue_tempo_t* other);
    virtual ~QueueTempo(); 
    QueueTempo* clone();

    int getId();
    int getPPQ();
    unsigned int getSkew();
    unsigned int getSkewBase();
    unsigned int getTempo();
    void setPPQ(int value);
    void setSkew(unsigned int value);
    void setSkewBase(unsigned int value);
    void setTempo(unsigned int value);
  
private:
    snd_seq_queue_tempo_t* m_Info;
};
  
  
class QueueTimer
{
	friend class MidiQueue;
	
public:
    QueueTimer();
    QueueTimer(snd_seq_queue_timer_t* other);
    virtual ~QueueTimer(); 
    QueueTimer* clone();
    
    int getQueueId();
    snd_seq_queue_timer_type_t getType();
    const snd_timer_id_t* getId();
    unsigned int getResolution();
    void setType(snd_seq_queue_timer_type_t value);
    void setId(snd_timer_id_t* value);
    void setResolution(unsigned int value);
  
private:
    snd_seq_queue_timer_t* m_Info;
};
 
  
class MidiQueue
{
public:
    MidiQueue(MidiClient* seq);
    MidiQueue(MidiClient* seq,  QueueInfo* info);
    MidiQueue(MidiClient* seq,  QString name);
    virtual ~MidiQueue();
    
    int getId() { return m_Id; }
    void start();
    void stop();
    void continueRunning();
    void clear();
    void setTickPosition(snd_seq_tick_time_t pos);
    void setRealTimePosition(snd_seq_real_time_t* pos);
    QueueInfo* getInfo();
    QueueStatus* getStatus();
    QueueTempo* getTempo();
    QueueTimer* getTimer();
    int getUsage();
    void setInfo(QueueInfo* value);
    void setTempo(QueueTempo* value);
    void setTimer(QueueTimer* value);
    void setUsage(int used);
    
private:
    int m_Id;
    MidiClient* m_MidiClient;
    QueueInfo*  m_Info;
    QueueTempo* m_Tempo;
    QueueTimer* m_Timer;
};

}
}

#endif //INCLUDED_QUEUE_H
