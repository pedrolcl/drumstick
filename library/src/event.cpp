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

#include "event.h"
 
namespace ALSA 
{
namespace Sequencer 
{

SequencerEvent::SequencerEvent() : QEvent(SequencerEventType)
{
    snd_seq_ev_clear( &m_event ); 
}

SequencerEvent::SequencerEvent(snd_seq_event_t* event) : QEvent(SequencerEventType)
{
    snd_seq_ev_clear( &m_event ); 
    m_event = *event;
}

SequencerEvent::SequencerEvent(const SequencerEvent& other) : QEvent(SequencerEventType)
{
    snd_seq_ev_clear( &m_event );  
    m_event = other.m_event;
}

SequencerEvent& 
SequencerEvent::operator=(const SequencerEvent& other)
{
    m_event = other.m_event;
    return *this;
}

bool SequencerEvent::isSubscription() const
{   
    return (m_event.type == SND_SEQ_EVENT_PORT_SUBSCRIBED || 
            m_event.type == SND_SEQ_EVENT_PORT_UNSUBSCRIBED );
}

bool SequencerEvent::isPort() const
{
    return (m_event.type == SND_SEQ_EVENT_PORT_START || 
            m_event.type == SND_SEQ_EVENT_PORT_EXIT || 
            m_event.type == SND_SEQ_EVENT_PORT_CHANGE );
}

bool SequencerEvent::isClient() const  
{   
    return (m_event.type == SND_SEQ_EVENT_CLIENT_START || 
            m_event.type == SND_SEQ_EVENT_CLIENT_EXIT || 
            m_event.type == SND_SEQ_EVENT_CLIENT_CHANGE );
}

bool SequencerEvent::isConnectionChange() const
{
    return (m_event.type == SND_SEQ_EVENT_PORT_START ||
            m_event.type == SND_SEQ_EVENT_PORT_EXIT ||
            m_event.type == SND_SEQ_EVENT_PORT_CHANGE ||
            m_event.type == SND_SEQ_EVENT_CLIENT_START ||
            m_event.type == SND_SEQ_EVENT_CLIENT_EXIT ||
            m_event.type == SND_SEQ_EVENT_CLIENT_CHANGE ||
            m_event.type == SND_SEQ_EVENT_PORT_SUBSCRIBED ||
            m_event.type == SND_SEQ_EVENT_PORT_UNSUBSCRIBED );
}

void SequencerEvent::setSequencerType(uchar eventType)
{
    m_event.type = eventType;
}

void SequencerEvent::setDestination(int client, int port)
{
    snd_seq_ev_set_dest(&m_event, client, port);
}

void SequencerEvent::setSource(int port)
{
    snd_seq_ev_set_source(&m_event, port);
}

void SequencerEvent::setSubscribers()
{
    snd_seq_ev_set_subs(&m_event);
}

void SequencerEvent::setBroadcast()
{
    snd_seq_ev_set_broadcast(&m_event);
}

void SequencerEvent::setDirect()
{
    snd_seq_ev_set_direct(&m_event);
}

void SequencerEvent::scheduleTick(int queue, int tick, bool relative)
{
    snd_seq_ev_schedule_tick(&m_event, queue, relative, tick);
}

void SequencerEvent::scheduleReal(int queue, ulong secs, ulong nanos, bool relative)
{
    snd_seq_real_time_t rtime;
    rtime.tv_sec = secs;
    rtime.tv_nsec = nanos;
    snd_seq_ev_schedule_real(&m_event, queue, relative, &rtime);
}

void SequencerEvent::setPriority(bool high)
{
    snd_seq_ev_set_priority(&m_event, high);
}

void SequencerEvent::setTag(int aTag)
{
    snd_seq_ev_set_tag(&m_event, aTag);
}

NoteEvent::NoteEvent(int ch, int key, int vel, int dur) : KeyEvent()
{
    snd_seq_ev_set_note(&m_event, ch, key, vel, dur);
}

NoteOnEvent::NoteOnEvent(int ch, int key, int vel) : KeyEvent()
{
    snd_seq_ev_set_noteon(&m_event, ch, key, vel);
}

NoteOffEvent::NoteOffEvent(int ch, int key, int vel) : KeyEvent()
{
    snd_seq_ev_set_noteoff(&m_event, ch, key, vel);
}

KeyPressEvent::KeyPressEvent(int ch, int key, int vel) : KeyEvent()
{
    snd_seq_ev_set_keypress(&m_event, ch, key, vel);
}

ControllerEvent::ControllerEvent(int ch, int cc, int val) : ChannelEvent()
{
    snd_seq_ev_set_controller(&m_event, ch, cc, val);
}

ProgramChangeEvent::ProgramChangeEvent(int ch, int val) : ChannelEvent()
{
    snd_seq_ev_set_pgmchange(&m_event, ch, val);
}

PitchBendEvent::PitchBendEvent(int ch, int val) : ChannelEvent()
{
    snd_seq_ev_set_pitchbend(&m_event, ch, val);
}

ChanPressEvent::ChanPressEvent(int ch, int val) : ChannelEvent()
{
    snd_seq_ev_set_chanpress(&m_event, ch, val);
}

SysExEvent::SysExEvent(const uint datalen,  char* dataptr) : SequencerEvent()
{
    snd_seq_ev_set_sysex(&m_event, datalen, dataptr);
}

SystemEvent::SystemEvent(int statusByte) : SequencerEvent()
{
    snd_seq_ev_set_fixed(&m_event);
    setSequencerType(statusByte);
}

QueueControlEvent::QueueControlEvent(int type, int queue, int value) : SequencerEvent()
{
    snd_seq_ev_set_queue_control(&m_event, type, queue, value);
}

ValueEvent::ValueEvent(int statusByte, int val) : SequencerEvent()
{
    snd_seq_ev_set_fixed(&m_event);
    setSequencerType(statusByte);
    setValue(val);
}

TempoEvent::TempoEvent(int queue, int tempo) : QueueControlEvent()
{
    snd_seq_ev_set_queue_tempo(&m_event, queue, tempo);
}

}
}
