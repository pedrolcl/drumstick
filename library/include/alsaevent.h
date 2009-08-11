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

#ifndef INCLUDED_ALSAEVENT_H
#define INCLUDED_ALSAEVENT_H

#include "aseqmmcommon.h"
#include <QEvent>

BEGIN_ALSASEQ_NAMESPACE

const QEvent::Type SequencerEventType = QEvent::Type(QEvent::User + 4154); // :-)

#define CLONE_EVENT_DECLARATION(T) virtual T* clone() { return new T(&m_event); }

class SequencerEvent : public QEvent
{
public:
    SequencerEvent();
    SequencerEvent(const SequencerEvent& other);
    SequencerEvent(snd_seq_event_t* event);
    virtual ~SequencerEvent() {}

    SequencerEvent& operator=(const SequencerEvent& other);
    bool isSubscription() const;
    bool isPort() const;
    bool isClient() const;
    bool isConnectionChange() const;
    void setSequencerType(const snd_seq_event_type_t eventType);
    snd_seq_event_type_t getSequencerType() const { return m_event.type; }
    void setDestination(const unsigned char client, const unsigned char port);
    void setSource(const unsigned char port);
    unsigned char getSourceClient() const { return m_event.source.client; }
    unsigned char getSourcePort() const { return m_event.source.port; }
    snd_seq_tick_time_t getTick() const { return m_event.time.tick; }
    unsigned int getRealTimeSecs() const { return m_event.time.time.tv_sec; }
    unsigned int getRealTimeNanos() const { return m_event.time.time.tv_nsec; }
    void setSubscribers();
    void setBroadcast();
    void setDirect();
    void scheduleTick(const int queue, const int tick, const bool relative);
    void scheduleReal(const int queue, const ulong secs, const ulong nanos, const bool relative);
    void setPriority(const bool high);
    unsigned char getTag() const { return m_event.tag; }
    void setTag(const unsigned char aTag);
    unsigned int getRaw32(const unsigned int n) const;
    void setRaw32(const unsigned int n, const unsigned int value);
    unsigned char getRaw8(const unsigned int n) const;
    void setRaw8(const unsigned int n, const unsigned char value);
    snd_seq_event_t* getHandle() { return &m_event; }
    int getEncodedLength();
    CLONE_EVENT_DECLARATION(SequencerEvent);

private:
    void free() __attribute__((deprecated));

protected:
    snd_seq_event_t m_event;
};


class ChannelEvent : public SequencerEvent
{
public:
    ChannelEvent() : SequencerEvent() {}
    ChannelEvent(snd_seq_event_t* event) : SequencerEvent(event) {}

    void setChannel(const MidiByte c) { m_event.data.note.channel = (c & 0xf); }
    int getChannel() const { return m_event.data.note.channel; }
};

class KeyEvent : public ChannelEvent
{
public:
    KeyEvent() : ChannelEvent() {}
    KeyEvent(snd_seq_event_t* event) : ChannelEvent(event) {}

    int getKey() const { return m_event.data.note.note; }
    void setKey(const MidiByte b) { m_event.data.note.note = b; }
    int getVelocity() const { return m_event.data.note.velocity; }
    void setVelocity(const MidiByte b) { m_event.data.note.velocity = b; }
};

class NoteEvent : public KeyEvent
{
public:
    NoteEvent() : KeyEvent() { m_event.type = SND_SEQ_EVENT_NOTE; }
    NoteEvent(snd_seq_event_t* event) : KeyEvent(event) {}
    NoteEvent(const int ch, const int key, const int vel, const int dur);

    ulong getDuration() const { return m_event.data.note.duration; }
    void setDuration(const ulong d) { m_event.data.note.duration = d; }
    CLONE_EVENT_DECLARATION(NoteEvent)
};

class NoteOnEvent : public KeyEvent
{
public:
    NoteOnEvent() : KeyEvent() { m_event.type = SND_SEQ_EVENT_NOTEON; }
    NoteOnEvent(snd_seq_event_t* event) : KeyEvent(event) {}
    NoteOnEvent(const int ch, const int key, const int vel);
    CLONE_EVENT_DECLARATION(NoteOnEvent)
};

class NoteOffEvent : public KeyEvent
{
public:
    NoteOffEvent() : KeyEvent() { m_event.type = SND_SEQ_EVENT_NOTEOFF; }
    NoteOffEvent(snd_seq_event_t* event) : KeyEvent(event) {}
    NoteOffEvent(const int ch, const int key, const int vel);
    CLONE_EVENT_DECLARATION(NoteOffEvent)
};

class KeyPressEvent : public KeyEvent
{
public:
    KeyPressEvent() : KeyEvent() { m_event.type = SND_SEQ_EVENT_KEYPRESS; }
    KeyPressEvent(snd_seq_event_t* event) : KeyEvent(event) {}
    KeyPressEvent(const int ch, const int key, const int vel);
    CLONE_EVENT_DECLARATION(KeyPressEvent)
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
    CLONE_EVENT_DECLARATION(ControllerEvent)
};

class ProgramChangeEvent : public ChannelEvent
{
public:
    ProgramChangeEvent() : ChannelEvent() { m_event.type = SND_SEQ_EVENT_PGMCHANGE; }
    ProgramChangeEvent(snd_seq_event_t* event) : ChannelEvent(event) {}
    ProgramChangeEvent(const int ch, const int val);

    int getValue() const { return m_event.data.control.value; }
    void setValue( const int v ) { m_event.data.control.value = v; }
    CLONE_EVENT_DECLARATION(ProgramChangeEvent)
};

class PitchBendEvent : public ChannelEvent
{
public:
    PitchBendEvent() : ChannelEvent() { m_event.type = SND_SEQ_EVENT_PITCHBEND; }
    PitchBendEvent(snd_seq_event_t* event) : ChannelEvent(event) {}
    PitchBendEvent(const int ch, const int val);

    int getValue() const { return m_event.data.control.value; }
    void setValue( const int v ) { m_event.data.control.value = v; }
    CLONE_EVENT_DECLARATION(PitchBendEvent)
};

class ChanPressEvent : public ChannelEvent
{
public:
    ChanPressEvent() : ChannelEvent() { m_event.type = SND_SEQ_EVENT_CHANPRESS; }
    ChanPressEvent(snd_seq_event_t* event) : ChannelEvent(event) {}
    ChanPressEvent( const int ch, const int val);

    int getValue() const { return m_event.data.control.value; }
    void setValue( const int v ) { m_event.data.control.value = v; }
    CLONE_EVENT_DECLARATION(ChanPressEvent)
};

class VariableEvent : public SequencerEvent
{
public:
    VariableEvent();
    VariableEvent(snd_seq_event_t* event);
    VariableEvent(const QByteArray& data);
    VariableEvent(const VariableEvent& other);
    VariableEvent(const unsigned int datalen, char* dataptr);
    VariableEvent& operator=(const VariableEvent& other);
    unsigned int getLength() const { return m_event.data.ext.len; }
    const char* getData() const { return static_cast<const char*>(m_event.data.ext.ptr); }
    CLONE_EVENT_DECLARATION(VariableEvent)
protected:
    QByteArray m_data;
};

class SysExEvent : public VariableEvent
{
public:
    SysExEvent();
    SysExEvent(snd_seq_event_t* event);
    SysExEvent(const QByteArray& data);
    SysExEvent(const SysExEvent& other);
    SysExEvent(const unsigned int datalen, char* dataptr);
    CLONE_EVENT_DECLARATION(SysExEvent)
};

class TextEvent : public VariableEvent
{
public:
    TextEvent();
    TextEvent(snd_seq_event_t* event);
    TextEvent(const QString& text, const int textType = 1);
    TextEvent(const TextEvent& other);
    TextEvent(const unsigned int datalen, char* dataptr);
    QString getText() const;
    int getTextType() const;
    CLONE_EVENT_DECLARATION(TextEvent)
protected:
    int m_textType;
};

class SystemEvent : public SequencerEvent
{
public:
    SystemEvent() : SequencerEvent() {}
    SystemEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
    SystemEvent(const int statusByte);
    CLONE_EVENT_DECLARATION(SystemEvent)
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
    CLONE_EVENT_DECLARATION(QueueControlEvent)
};

class ValueEvent : public SequencerEvent
{
public:
    ValueEvent() : SequencerEvent() {}
    ValueEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
    ValueEvent(const int statusByte, const int val);

    int getValue() const { return m_event.data.control.value; }
    void setValue( const int v ) { m_event.data.control.value = v; }
    CLONE_EVENT_DECLARATION(ValueEvent)
};

class TempoEvent : public QueueControlEvent
{
public:
    TempoEvent() : QueueControlEvent() {}
    TempoEvent(snd_seq_event_t* event) : QueueControlEvent(event) {}
    TempoEvent(const int queue, const int tempo);
    CLONE_EVENT_DECLARATION(TempoEvent)
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
    CLONE_EVENT_DECLARATION(SubscriptionEvent)
};

class ClientEvent : public SequencerEvent
{
public:
    ClientEvent() : SequencerEvent() {}
    ClientEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
    int getClient() const { return m_event.data.addr.client; }
    CLONE_EVENT_DECLARATION(ClientEvent)
};

class PortEvent : public ClientEvent
{
public:
    PortEvent() : ClientEvent() {}
    PortEvent(snd_seq_event_t* event) : ClientEvent(event) {}
    int getPort() const { return m_event.data.addr.port; }
    CLONE_EVENT_DECLARATION(PortEvent)
};

class RemoveEvents
{
public:
    friend class MidiClient;

public:
    RemoveEvents();
    RemoveEvents(const RemoveEvents& other);
    RemoveEvents(snd_seq_remove_events_t* other);
    virtual ~RemoveEvents();
    RemoveEvents* clone();
    RemoveEvents& operator=(const RemoveEvents& other);
    int getSizeOfInfo() const;

    int getChannel();
    unsigned int getCondition();
    const snd_seq_addr_t* getDest();
    int getEventType();
    int getQueue();
    int getTag();
    const snd_seq_timestamp_t* getTime();
    void setChannel(int chan);
    void setCondition(unsigned int cond);
    void setDest(const snd_seq_addr_t* dest);
    void setEventType(int type);
    void setQueue(int queue);
    void setTag(int tag);
    void setTime(const snd_seq_timestamp_t* time);

private:
    snd_seq_remove_events_t* m_Info;
};

class MIDICodec : public QObject
{
    Q_OBJECT
public:
    MIDICodec(int bufsize, QObject* parent = 0);
    ~MIDICodec();

    void init();
    long decode(unsigned char *buf,
                long count,
                const snd_seq_event_t *ev);
    long encode(const unsigned char *buf,
                long count,
                snd_seq_event_t *ev);
    long encode(int c,
                snd_seq_event_t *ev);
    void enableRunningStatus(bool enable);
    void resetEncoder();
    void resetDecoder();
    void resizeBuffer(int bufsize);
private:
    snd_midi_event_t* m_Info;
};

END_ALSASEQ_NAMESPACE

#endif //INCLUDED_ALSAEVENT_H
