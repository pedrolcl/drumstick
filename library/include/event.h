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
 
#ifndef INCLUDED_EVENT_H
#define INCLUDED_EVENT_H

#include "commons.h"
#include <qevent.h>

namespace ALSA 
{
namespace Sequencer 
{

#define SequencerEventType (QEvent::User + 4154) // :-)

class SequencerEvent : public QCustomEvent
{
public:
    SequencerEvent();
    SequencerEvent(snd_seq_event_t* event);
    ~SequencerEvent() {}
    
    bool isSubscription();
    bool isPort();
    bool isClient();
    bool isConnectionChange();
    void setType(snd_seq_event_type_t eventType);
    snd_seq_event_type_t getType();
    void setDestination(int client, int port);
    void setSource(int port);
    int getSourceClient();
    int getSourcePort();
    snd_seq_tick_time_t getTick();
    unsigned int getRealTimeSecs();
    unsigned int getRealTimeNanos();
    void setSubscribers();
    void setBroadcast();
    void setDirect();
    void scheduleTick(int queue, int tick, bool relative);
    void scheduleReal(int queue, ulong secs, ulong nanos, bool relative);
    void setPriority(bool high);
    int getTag();
    void setTag(int aTag);
    snd_seq_event_t *getEvent();

protected:
    snd_seq_event_t m_event;
};

class ChannelEvent : public SequencerEvent
{
public:
	ChannelEvent() : SequencerEvent() {}
	ChannelEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
	
    void setChannel(MidiByte c) { m_event.data.note.channel = (c & 0xf); }
    int getChannel() { return m_event.data.note.channel; }
};
  
class KeyEvent : public ChannelEvent
{
public:
    KeyEvent() : ChannelEvent() {}
    KeyEvent(snd_seq_event_t* event) : ChannelEvent(event) {}
    
    int getKey() { return m_event.data.note.note; }
    void setKey(MidiByte b) { m_event.data.note.note = b; }
    int getVelocity() { return m_event.data.note.velocity; }
    void setVelocity(MidiByte b) { m_event.data.note.velocity = b; }
};   
  
class NoteEvent : public KeyEvent
{
public:
	NoteEvent() : KeyEvent() {}
	NoteEvent(snd_seq_event_t* event) : KeyEvent(event) {}
    NoteEvent(int ch, int key, int vel, int dur);

    ulong getDuration() { return m_event.data.note.duration; }
    void setDuration(ulong d) { m_event.data.note.duration = d; }
};
  
class NoteOnEvent : public KeyEvent 
{
public:
	NoteOnEvent() : KeyEvent() {}
	NoteOnEvent(snd_seq_event_t* event) : KeyEvent(event) {}
    NoteOnEvent(int ch, int key, int vel);
};
  
class NoteOffEvent : public KeyEvent 
{
public:
	NoteOffEvent() : KeyEvent() {}
	NoteOffEvent(snd_seq_event_t* event) : KeyEvent(event) {}
    NoteOffEvent(int ch, int key, int vel);
};
  
class KeyPressEvent : public KeyEvent 
{
public:
	KeyPressEvent() : KeyEvent() {}
	KeyPressEvent(snd_seq_event_t* event) : KeyEvent(event) {}
    KeyPressEvent(int ch, int key, int vel);
};
  
class ControllerEvent : public ChannelEvent
{
public:
	ControllerEvent() : ChannelEvent() {}
	ControllerEvent(snd_seq_event_t* event) : ChannelEvent(event) {}
	ControllerEvent(int ch, int cc, int val);
    
    uint getParam() { return m_event.data.control.param; }
    void setParam( uint p ) { m_event.data.control.param = p; }
    int getValue() { return m_event.data.control.value; }
    void setValue( int v ) { m_event.data.control.value = v; }
};
  
class ProgramChangeEvent : public ChannelEvent
{
public:
	ProgramChangeEvent() : ChannelEvent() {}
	ProgramChangeEvent(snd_seq_event_t* event) : ChannelEvent(event) {}
    ProgramChangeEvent(int ch, int val);
    
    int getValue() { return m_event.data.control.value; }
    void setValue( int v ) { m_event.data.control.value = v; }
};
  
class PitchBendEvent : public ChannelEvent
{
public:
	PitchBendEvent() : ChannelEvent() {}
	PitchBendEvent(snd_seq_event_t* event) : ChannelEvent(event) {}
	PitchBendEvent(int ch, int val);

    int getValue() { return m_event.data.control.value; }
    void setValue( int v ) { m_event.data.control.value = v; }
};
  
class ChanPressEvent : public ChannelEvent
{
public:
	ChanPressEvent() : ChannelEvent() {}
	ChanPressEvent(snd_seq_event_t* event) : ChannelEvent(event) {}
    ChanPressEvent(int ch, int val);
    
    int getValue() { return m_event.data.control.value; }
    void setValue( int v ) { m_event.data.control.value = v; }
};
  
class SysExEvent : public SequencerEvent
{
public:
	SysExEvent() : SequencerEvent() {}
	SysExEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
    SysExEvent(uint datalen, char* dataptr);
    
    uint getLength() { return m_event.data.ext.len; }
    uchar* getData() { return static_cast<uchar*>(m_event.data.ext.ptr); }
};
 
class SystemEvent : public SequencerEvent
{
public:
	SystemEvent() : SequencerEvent() {}
	SystemEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
    SystemEvent(int statusByte);
};

class QueueControlEvent : public SequencerEvent
{
public:
	QueueControlEvent() : SequencerEvent() {}
	QueueControlEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
	QueueControlEvent(int type, int queue, int value);
	int getQueue() { return m_event.data.queue.queue; }
	void setQueue(uchar q) { m_event.data.queue.queue = q; }
	int getValue() { return m_event.data.queue.param.value; }
	void setValue(int val) { m_event.data.queue.param.value = val; }
	uint getPosition() { return m_event.data.queue.param.position; }
	void setPosition(uint pos) { m_event.data.queue.param.position = pos; }
	snd_seq_tick_time_t getTickTime() { return m_event.data.queue.param.time.tick; }
	void setTickTime(snd_seq_tick_time_t t) { m_event.data.queue.param.time.tick = t; }
	uint getSkewBase() { return m_event.data.queue.param.skew.base;  }
	void setSkewBase(uint base) { m_event.data.queue.param.skew.base = base; }
	uint getSkewValue() { return m_event.data.queue.param.skew.value;  }
	void setSkewValue(uint val) {m_event.data.queue.param.skew.value = val; }
};
  
class ValueEvent : public SequencerEvent
{
public:
	ValueEvent() : SequencerEvent() {}
	ValueEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
	ValueEvent(int statusByte, int val);
    
    int getValue() { return m_event.data.control.value; }
    void setValue( int v ) { m_event.data.control.value = v; }
};

class TempoEvent : public QueueControlEvent
{
public:
	TempoEvent() : QueueControlEvent() {}
	TempoEvent(snd_seq_event_t* event) : QueueControlEvent(event) {}
    TempoEvent(int queue, int tempo);
};

class SubscriptionEvent : public SequencerEvent
{
public:
	SubscriptionEvent() : SequencerEvent() {}
	SubscriptionEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
	
	bool subscribed() { return (m_event.type == SND_SEQ_EVENT_PORT_SUBSCRIBED); }
	bool unsubscribed() { return (m_event.type == SND_SEQ_EVENT_PORT_UNSUBSCRIBED); }
	int getSenderClient() { return m_event.data.connect.sender.client; }
	int getSenderPort() { return m_event.data.connect.sender.port; }
	int getDestClient() { return m_event.data.connect.dest.client; }
	int getDestPort() { return m_event.data.connect.dest.port; }
};

class ClientEvent : public SequencerEvent
{
public:
	ClientEvent() : SequencerEvent() {}
	ClientEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
	int getClient() { return m_event.data.addr.client; }
};

class PortEvent : public ClientEvent
{
public:
	PortEvent() : ClientEvent() {}
	PortEvent(snd_seq_event_t* event) : ClientEvent(event) {}
	int getPort() { return m_event.data.addr.port; }
};

}
}

#endif //INCLUDED_EVENT_H
