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
#include <QEvent>

namespace ALSA 
{
namespace Sequencer 
{

const QEvent::Type SequencerEventType = QEvent::Type(QEvent::User + 4154); // :-)

class SequencerEvent : public QEvent
{
public:
    SequencerEvent();
    SequencerEvent(const SequencerEvent& other);
    SequencerEvent(snd_seq_event_t* event);
    ~SequencerEvent() {}
    
    SequencerEvent& operator=(const SequencerEvent& other);
    bool isSubscription() const;
    bool isPort() const;
    bool isClient() const;
    bool isConnectionChange() const;
    void setType(const snd_seq_event_type_t eventType);
    snd_seq_event_type_t getType() const { return m_event.type; }
    void setDestination(const int client, const int port);
    void setSource(const int port);
    int getSourceClient() const { return m_event.source.client; }
    int getSourcePort() const { return m_event.source.port; }
    snd_seq_tick_time_t getTick() const { return m_event.time.tick; }
    unsigned int getRealTimeSecs() const { return m_event.time.time.tv_sec; }
    unsigned int getRealTimeNanos() const { return m_event.time.time.tv_nsec; }
    void setSubscribers();
    void setBroadcast();
    void setDirect();
    void scheduleTick(const int queue, const int tick, const bool relative);
    void scheduleReal(const int queue, const ulong secs, const ulong nanos, const bool relative);
    void setPriority(const bool high);
    int getTag() const { return m_event.tag; }
    void setTag(const int aTag);
    snd_seq_event_t* getEvent() { return &m_event; }

protected:
    snd_seq_event_t m_event;
};

class ChannelEvent : public SequencerEvent
{
public:
	ChannelEvent() : SequencerEvent() {}
	ChannelEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
	
    void setChannel(const MidiByte c) { m_event.data.note.channel = (c & 0xf); }
    int getChannel() { return m_event.data.note.channel; }
};
  
class KeyEvent : public ChannelEvent
{
public:
    KeyEvent() : ChannelEvent() {}
    KeyEvent(snd_seq_event_t* event) : ChannelEvent(event) {}
    
    int getKey()  const { return m_event.data.note.note; }
    void setKey(const MidiByte b) { m_event.data.note.note = b; }
    int getVelocity() const { return m_event.data.note.velocity; }
    void setVelocity(const MidiByte b) { m_event.data.note.velocity = b; }
};   
  
class NoteEvent : public KeyEvent
{
public:
	NoteEvent() : KeyEvent() {}
	NoteEvent(snd_seq_event_t* event) : KeyEvent(event) {}
    NoteEvent(const int ch, const int key, const int vel, const int dur);

    ulong getDuration() const { return m_event.data.note.duration; }
    void setDuration(const ulong d) { m_event.data.note.duration = d; }
};
  
class NoteOnEvent : public KeyEvent 
{
public:
	NoteOnEvent() : KeyEvent() {}
	NoteOnEvent(snd_seq_event_t* event) : KeyEvent(event) {}
    NoteOnEvent(const int ch, const int key, const int vel);
};
  
class NoteOffEvent : public KeyEvent 
{
public:
	NoteOffEvent() : KeyEvent() {}
	NoteOffEvent(snd_seq_event_t* event) : KeyEvent(event) {}
    NoteOffEvent(const int ch, const int key, const int vel);
};
  
class KeyPressEvent : public KeyEvent 
{
public:
	KeyPressEvent() : KeyEvent() {}
	KeyPressEvent(snd_seq_event_t* event) : KeyEvent(event) {}
    KeyPressEvent(const int ch, const int key, const int vel);
};
  
class ControllerEvent : public ChannelEvent
{
public:
	ControllerEvent() : ChannelEvent() {}
	ControllerEvent(snd_seq_event_t* event) : ChannelEvent(event) {}
	ControllerEvent(const int ch, const int cc, const int val);
    
    uint getParam() const { return m_event.data.control.param; }
    void setParam( const uint p ) { m_event.data.control.param = p; }
    int getValue() const { return m_event.data.control.value; }
    void setValue( const int v ) { m_event.data.control.value = v; }
};
  
class ProgramChangeEvent : public ChannelEvent
{
public:
	ProgramChangeEvent() : ChannelEvent() {}
	ProgramChangeEvent(snd_seq_event_t* event) : ChannelEvent(event) {}
    ProgramChangeEvent(const int ch, const int val);
    
    int getValue() const { return m_event.data.control.value; }
    void setValue( const int v ) { m_event.data.control.value = v; }
};
  
class PitchBendEvent : public ChannelEvent
{
public:
	PitchBendEvent() : ChannelEvent() {}
	PitchBendEvent(snd_seq_event_t* event) : ChannelEvent(event) {}
	PitchBendEvent(const int ch, const int val);

    int getValue() const { return m_event.data.control.value; }
    void setValue( const int v ) { m_event.data.control.value = v; }
};
  
class ChanPressEvent : public ChannelEvent
{
public:
	ChanPressEvent() : ChannelEvent() {}
	ChanPressEvent(snd_seq_event_t* event) : ChannelEvent(event) {}
    ChanPressEvent( const int ch, const int val);
    
    int getValue() const { return m_event.data.control.value; }
    void setValue( const int v ) { m_event.data.control.value = v; }
};
  
class SysExEvent : public SequencerEvent
{
public:
	SysExEvent() : SequencerEvent() {}
	SysExEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
    SysExEvent(const uint datalen, char* dataptr);
    
    uint getLength() const { return m_event.data.ext.len; }
    const uchar* getData() const { return static_cast<const uchar*>(m_event.data.ext.ptr); }
};
 
class SystemEvent : public SequencerEvent
{
public:
	SystemEvent() : SequencerEvent() {}
	SystemEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
    SystemEvent(const int statusByte);
};

class QueueControlEvent : public SequencerEvent
{
public:
	QueueControlEvent() : SequencerEvent() {}
	QueueControlEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
	QueueControlEvent(const int type, const int queue, const int value);
	int getQueue() const { return m_event.data.queue.queue; }
	void setQueue(const uchar q) { m_event.data.queue.queue = q; }
	int getValue() const { return m_event.data.queue.param.value; }
	void setValue(const int val) { m_event.data.queue.param.value = val; }
	uint getPosition() const { return m_event.data.queue.param.position; }
	void setPosition(const uint pos) { m_event.data.queue.param.position = pos; }
	snd_seq_tick_time_t getTickTime() const { return m_event.data.queue.param.time.tick; }
	void setTickTime(const snd_seq_tick_time_t t) { m_event.data.queue.param.time.tick = t; }
	uint getSkewBase() const { return m_event.data.queue.param.skew.base;  }
	void setSkewBase(const uint base) { m_event.data.queue.param.skew.base = base; }
	uint getSkewValue() const { return m_event.data.queue.param.skew.value;  }
	void setSkewValue(const uint val) {m_event.data.queue.param.skew.value = val; }
};
  
class ValueEvent : public SequencerEvent
{
public:
	ValueEvent() : SequencerEvent() {}
	ValueEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
	ValueEvent(const int statusByte, const int val);
    
    int getValue() const { return m_event.data.control.value; }
    void setValue( const int v ) { m_event.data.control.value = v; }
};

class TempoEvent : public QueueControlEvent
{
public:
	TempoEvent() : QueueControlEvent() {}
	TempoEvent(snd_seq_event_t* event) : QueueControlEvent(event) {}
    TempoEvent(const int queue, const int tempo);
};

class SubscriptionEvent : public SequencerEvent
{
public:
	SubscriptionEvent() : SequencerEvent() {}
	SubscriptionEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
	
	bool subscribed() const { return (m_event.type == SND_SEQ_EVENT_PORT_SUBSCRIBED); }
	bool unsubscribed() const { return (m_event.type == SND_SEQ_EVENT_PORT_UNSUBSCRIBED); }
	int getSenderClient() const { return m_event.data.connect.sender.client; }
	int getSenderPort() const { return m_event.data.connect.sender.port; }
	int getDestClient() const { return m_event.data.connect.dest.client; }
	int getDestPort() const { return m_event.data.connect.dest.port; }
};

class ClientEvent : public SequencerEvent
{
public:
	ClientEvent() : SequencerEvent() {}
	ClientEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
	int getClient() const { return m_event.data.addr.client; }
};

class PortEvent : public ClientEvent
{
public:
	PortEvent() : ClientEvent() {}
	PortEvent(snd_seq_event_t* event) : ClientEvent(event) {}
	int getPort() const { return m_event.data.addr.port; }
};

}
}

#endif //INCLUDED_EVENT_H
