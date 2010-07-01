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

#ifndef DRUMSTICK_ALSAEVENT_H
#define DRUMSTICK_ALSAEVENT_H

#include "drumstickcommon.h"
#include <QEvent>

/**
 * @file alsaevent.h
 * Classes managing ALSA Sequencer events.
 *
 * @defgroup ALSAEvent ALSA Sequencer Events
 * @{
 */

namespace drumstick {

/**
 * Constant SequencerEventType is the QEvent::type() of any SequencerEvent
 * object to be used to check the argument in QObject::customEvent().
 */
const QEvent::Type SequencerEventType = QEvent::Type(QEvent::User + 4154); // :-)

/**
 * Macro to declare a virtual clone() method for SequencerEvent and derived classes.
 */
#define CLONE_EVENT_DECLARATION(T) virtual T* clone() { return new T(&m_event); }

/**
 * Base class for the event's hierarchy
 *
 * All event classes share this base class. It provides several common
 * properties and methods.
 */
class DRUMSTICK_EXPORT SequencerEvent : public QEvent
{
public:
    SequencerEvent();
    SequencerEvent(const SequencerEvent& other);
    SequencerEvent(snd_seq_event_t* event);
    /** Destructor */
    virtual ~SequencerEvent() {}

    SequencerEvent& operator=(const SequencerEvent& other);
    void setSequencerType(const snd_seq_event_type_t eventType);
    /**
     * Gets the sequencer event type.
     * @return The sequencer event type.
     * @see setSequencerType()
     */
    snd_seq_event_type_t getSequencerType() const { return m_event.type; }
    void setDestination(const unsigned char client, const unsigned char port);
    void setSource(const unsigned char port);
    /**
     * Gets the source client id.
     * @return The source client id.
     * @see setSource()
     */
    unsigned char getSourceClient() const { return m_event.source.client; }
    /**
     * Gets the source port id.
     * @return The source port id.
     * @see setSource()
     */
    unsigned char getSourcePort() const { return m_event.source.port; }
    /**
     * Gets the tick time of the event.
     * @return The tick time.
     * @see scheduleTick()
     */
    snd_seq_tick_time_t getTick() const { return m_event.time.tick; }
    /**
     * Gets the seconds of the event's real time.
     * @return The seconds of the time.
     * @see scheduleReal(), getRealTimeNanos()
     */
    unsigned int getRealTimeSecs() const { return m_event.time.time.tv_sec; }
    /**
     * Gets the nanoseconds of the event's real time.
     * @return The nanoseconds of the time.
     * @see scheduleReal(), getRealTimeSecs()
     */
    unsigned int getRealTimeNanos() const { return m_event.time.time.tv_nsec; }
    void setSubscribers();
    void setBroadcast();
    void setDirect();
    void scheduleTick(const int queue, const int tick, const bool relative);
    void scheduleReal(const int queue, const ulong secs, const ulong nanos, const bool relative);
    void setPriority(const bool high);
    /**
     * Gets the tag of the event
     * @return The event's tag
     * @see setTag()
     */
    unsigned char getTag() const { return m_event.tag; }
    void setTag(const unsigned char aTag);
    unsigned int getRaw32(const unsigned int n) const;
    void setRaw32(const unsigned int n, const unsigned int value);
    unsigned char getRaw8(const unsigned int n) const;
    void setRaw8(const unsigned int n, const unsigned char value);
    /**
     * Gets the handle of the event
     * @return The event's handle
     */
    snd_seq_event_t* getHandle() { return &m_event; }
    int getEncodedLength();

    static bool isSubscription(const SequencerEvent* event);
    static bool isPort(const SequencerEvent* event);
    static bool isClient(const SequencerEvent* event);
    static bool isConnectionChange(const SequencerEvent* event);
    static bool isChannel(const SequencerEvent* event);

    /** Clone this object returning a pointer to the new object */
    CLONE_EVENT_DECLARATION(SequencerEvent);

protected:
    void free() __attribute__((deprecated));

    /**
     * ALSA sequencer event record.
     * @see http://www.alsa-project.org/alsa-doc/alsa-lib/structsnd__seq__event.html
     */
    snd_seq_event_t m_event;
};

/**
 * Base class for the events having a Channel property
 */
class DRUMSTICK_EXPORT ChannelEvent : public SequencerEvent
{
public:
    /** Default constructor */
    ChannelEvent() : SequencerEvent() {}
    /** Constructor from an ALSA event record */
    ChannelEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
    /**
     * Sets the channel of the event
     * @param c A channel, between 0 and 15.
     * @see getChannel()
     */
    void setChannel(const MidiByte c) { m_event.data.note.channel = (c & 0xf); }
    /**
     * Gets the event's channel
     * @return The event's channel
     * @see setChannel()
     */
    int getChannel() const { return m_event.data.note.channel; }
};

/**
 * Base class for the events having Key and Velocity properties.
 */
class DRUMSTICK_EXPORT KeyEvent : public ChannelEvent
{
public:
    /** Default constructor */
    KeyEvent() : ChannelEvent() {}
    /** Constructor from an ALSA event record */
    KeyEvent(snd_seq_event_t* event) : ChannelEvent(event) {}
    /**
     * Gets the MIDI note of this event.
     * @return The event's MIDI note.
     * @see setKey()
     */
    int getKey() const { return m_event.data.note.note; }
    /**
     * Sets the MIDI note of this event.
     * @param b A MIDI note, between 0 and 127.
     * @see getKey()
     */
    void setKey(const MidiByte b) { m_event.data.note.note = b; }
    /**
     * Gets the note velocity of this event.
     * @return The event's note velocity.
     * @see setVelocity()
     */
    int getVelocity() const { return m_event.data.note.velocity; }
    /**
     * Sets the note velocity of this event.
     * @param b A velocity value, between 0 and 127.
     * @see getVelocity()
     */
    void setVelocity(const MidiByte b) { m_event.data.note.velocity = b; }
};

/**
 * Class representing a note event with duration
 *
 * Note events are converted into two MIDI events, a note-on and a note-off
 * over the wire.
 */
class DRUMSTICK_EXPORT NoteEvent : public KeyEvent
{
public:
    /** Default constructor */
    NoteEvent() : KeyEvent() { m_event.type = SND_SEQ_EVENT_NOTE; }
    /** Constructor from an ALSA event record */
    NoteEvent(snd_seq_event_t* event) : KeyEvent(event) {}
    NoteEvent(const int ch, const int key, const int vel, const int dur);
    /**
     * Gets the note's duration
     * @return The duration of the event
     * @see setDuration()
     */
    ulong getDuration() const { return m_event.data.note.duration; }
    /**
     * Sets the note's duration
     * @param d The duration of the event
     * @see getDuration()
     */
    void setDuration(const ulong d) { m_event.data.note.duration = d; }
    /** Clone this object returning a pointer to the new object */
    CLONE_EVENT_DECLARATION(NoteEvent)
};

/**
 * Event representing a note-on MIDI event
 */
class DRUMSTICK_EXPORT NoteOnEvent : public KeyEvent
{
public:
    /** Default constructor */
    NoteOnEvent() : KeyEvent() { m_event.type = SND_SEQ_EVENT_NOTEON; }
    /** Constructor from an ALSA event record */
    NoteOnEvent(snd_seq_event_t* event) : KeyEvent(event) {}
    NoteOnEvent(const int ch, const int key, const int vel);
    /** Clone this object returning a pointer to the new object */
    CLONE_EVENT_DECLARATION(NoteOnEvent)
};

/**
 * Event representing a note-off MIDI event
 */
class DRUMSTICK_EXPORT NoteOffEvent : public KeyEvent
{
public:
    /** Default constructor */
    NoteOffEvent() : KeyEvent() { m_event.type = SND_SEQ_EVENT_NOTEOFF; }
    /** Constructor from an ALSA event record */
    NoteOffEvent(snd_seq_event_t* event) : KeyEvent(event) {}
    NoteOffEvent(const int ch, const int key, const int vel);
    /** Clone this object returning a pointer to the new object */
    CLONE_EVENT_DECLARATION(NoteOffEvent)
};

/**
 * Event representing a MIDI key pressure, or polyphonic after-touch event
 */
class DRUMSTICK_EXPORT KeyPressEvent : public KeyEvent
{
public:
    /** Default constructor */
    KeyPressEvent() : KeyEvent() { m_event.type = SND_SEQ_EVENT_KEYPRESS; }
    /** Constructor from an ALSA event record */
    KeyPressEvent(snd_seq_event_t* event) : KeyEvent(event) {}
    KeyPressEvent(const int ch, const int key, const int vel);
    /** Clone this object returning a pointer to the new object */
    CLONE_EVENT_DECLARATION(KeyPressEvent)
};

/**
 * Event representing a MIDI control change event
 */
class DRUMSTICK_EXPORT ControllerEvent : public ChannelEvent
{
public:
    /** Default constructor */
    ControllerEvent() : ChannelEvent() {}
    /** Constructor from an ALSA event record */
    ControllerEvent(snd_seq_event_t* event) : ChannelEvent(event) {}
    ControllerEvent(const int ch, const int cc, const int val);
    /**
     * Gets the controller event's parameter.
     * @return The controller event's parameter.
     * @see setParam()
     */
    uint getParam() const { return m_event.data.control.param; }
    /**
     * Sets the controller event's parameter.
     * @param p The controller event's parameter.
     * @see getParam()
     */
    void setParam( const uint p ) { m_event.data.control.param = p; }
    /**
     * Gets the controller event's value.
     * @return The controller event's value.
     * @see setValue()
     */
    int getValue() const { return m_event.data.control.value; }
    /**
     * Sets the controller event's value.
     * @param v The controller event's value.
     * @see getValue()
     */
    void setValue( const int v ) { m_event.data.control.value = v; }
    /** Clone this object returning a pointer to the new object */
    CLONE_EVENT_DECLARATION(ControllerEvent)
};

/**
 * Event representing a MIDI program change event
 */
class DRUMSTICK_EXPORT ProgramChangeEvent : public ChannelEvent
{
public:
    /** Default constructor */
    ProgramChangeEvent() : ChannelEvent() { m_event.type = SND_SEQ_EVENT_PGMCHANGE; }
    /** Constructor from an ALSA event record */
    ProgramChangeEvent(snd_seq_event_t* event) : ChannelEvent(event) {}
    ProgramChangeEvent(const int ch, const int val);
    /** Gets the MIDI program number */
    int getValue() const { return m_event.data.control.value; }
    /** Sets the MIDI program number */
    void setValue( const int v ) { m_event.data.control.value = v; }
    /** Clone this object returning a pointer to the new object */
    CLONE_EVENT_DECLARATION(ProgramChangeEvent)
};

/**
 * Event representing a MIDI bender, or pitch wheel event
 */
class DRUMSTICK_EXPORT PitchBendEvent : public ChannelEvent
{
public:
    /** Default constructor */
    PitchBendEvent() : ChannelEvent() { m_event.type = SND_SEQ_EVENT_PITCHBEND; }
    /** Constructor from an ALSA event record */
    PitchBendEvent(snd_seq_event_t* event) : ChannelEvent(event) {}
    PitchBendEvent(const int ch, const int val);
    /** Gets the MIDI pitch bend value, zero centered from -8192 to 8191 */
    int getValue() const { return m_event.data.control.value; }
    /** Sets the MIDI pitch bend value, zero centered from -8192 to 8191  */
    void setValue( const int v ) { m_event.data.control.value = v; }
    /** Clone this object returning a pointer to the new object */
    CLONE_EVENT_DECLARATION(PitchBendEvent)
};

/**
 * Event representing a MIDI channel pressure or after-touch event
 */
class DRUMSTICK_EXPORT ChanPressEvent : public ChannelEvent
{
public:
    /** Default constructor */
    ChanPressEvent() : ChannelEvent() { m_event.type = SND_SEQ_EVENT_CHANPRESS; }
    /** Constructor from an ALSA event record */
    ChanPressEvent(snd_seq_event_t* event) : ChannelEvent(event) {}
    ChanPressEvent( const int ch, const int val);
    /** Gets the channel aftertouch value */
    int getValue() const { return m_event.data.control.value; }
    /** Sets the channel aftertouch value */
    void setValue( const int v ) { m_event.data.control.value = v; }
    /** Clone this object returning a pointer to the new object */
    CLONE_EVENT_DECLARATION(ChanPressEvent)
};

/**
 * Base class for variable length events
 */
class DRUMSTICK_EXPORT VariableEvent : public SequencerEvent
{
public:
    VariableEvent();
    VariableEvent(snd_seq_event_t* event);
    VariableEvent(const QByteArray& data);
    VariableEvent(const VariableEvent& other);
    VariableEvent(const unsigned int datalen, char* dataptr);
    VariableEvent& operator=(const VariableEvent& other);
    /** Gets the data length */
    unsigned int getLength() const { return m_event.data.ext.len; }
    /** Gets the data pointer */
    const char* getData() const { return static_cast<const char*>(m_event.data.ext.ptr); }
    /** Clone this object returning a pointer to the new object */
    CLONE_EVENT_DECLARATION(VariableEvent)
protected:
    QByteArray m_data;
};

/**
 * Event representing a MIDI system exclusive event
 */
class DRUMSTICK_EXPORT SysExEvent : public VariableEvent
{
public:
    SysExEvent();
    SysExEvent(snd_seq_event_t* event);
    SysExEvent(const QByteArray& data);
    SysExEvent(const SysExEvent& other);
    SysExEvent(const unsigned int datalen, char* dataptr);
    /** Clone this object returning a pointer to the new object */
    CLONE_EVENT_DECLARATION(SysExEvent)
};

/**
 * Event representing a SMF text event
 *
 * This event type is not intended to be transmitted over the wire to an
 * external device, but it is useful for sequencer programs or MIDI applications
 */
class DRUMSTICK_EXPORT TextEvent : public VariableEvent
{
public:
    TextEvent();
    TextEvent(snd_seq_event_t* event);
    explicit TextEvent(const QString& text, const int textType = 1);
    TextEvent(const TextEvent& other);
    TextEvent(const unsigned int datalen, char* dataptr);
    QString getText() const;
    int getTextType() const;
    /** Clone this object returning a pointer to the new object */
    CLONE_EVENT_DECLARATION(TextEvent)
protected:
    int m_textType;
};

/**
 * Generic event
 */
class DRUMSTICK_EXPORT SystemEvent : public SequencerEvent
{
public:
    /** Default constructor */
    SystemEvent() : SequencerEvent() {}
    /** Constructor from an ALSA event record */
    SystemEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
    SystemEvent(const snd_seq_event_type_t type);
    /** Clone this object returning a pointer to the new object */
    CLONE_EVENT_DECLARATION(SystemEvent)
};

/**
 * ALSA Event representing a queue control command
 *
 * This event is used to schedule changes to the ALSA queues
 */
class DRUMSTICK_EXPORT QueueControlEvent : public SequencerEvent
{
public:
    /** Default constructor */
    QueueControlEvent() : SequencerEvent() {}
    /** Constructor from an ALSA event record */
    QueueControlEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
    QueueControlEvent(const snd_seq_event_type_t type, const int queue, const int value);
    /** Gets the queue number */
    int getQueue() const { return m_event.data.queue.queue; }
    /** Sets the queue number */
    void setQueue(const uchar q) { m_event.data.queue.queue = q; }
    /** Gets the event's value */
    int getValue() const { return m_event.data.queue.param.value; }
    /** Sets the event's value */
    void setValue(const int val) { m_event.data.queue.param.value = val; }
    /** Gets the queue position */
    uint getPosition() const { return m_event.data.queue.param.position; }
    /** Sets the queue position */
    void setPosition(const uint pos) { m_event.data.queue.param.position = pos; }
    /** Gets the musical time in ticks */
    snd_seq_tick_time_t getTickTime() const { return m_event.data.queue.param.time.tick; }
    /** Sets the musical time in ticks */
    void setTickTime(const snd_seq_tick_time_t t) { m_event.data.queue.param.time.tick = t; }
    /** Gets the skew base */
    uint getSkewBase() const { return m_event.data.queue.param.skew.base;  }
    /** Sets the skew base, should be 65536 */
    void setSkewBase(const uint base) { m_event.data.queue.param.skew.base = base; }
    /** Gets the skew value */
    uint getSkewValue() const { return m_event.data.queue.param.skew.value;  }
    /** Sets the skew value */
    void setSkewValue(const uint val) {m_event.data.queue.param.skew.value = val; }
    /** Clone this object returning a pointer to the new object */
    CLONE_EVENT_DECLARATION(QueueControlEvent)
};

/**
 * Generic event having a value property
 */
class DRUMSTICK_EXPORT ValueEvent : public SequencerEvent
{
public:
    /** Default constructor */
    ValueEvent() : SequencerEvent() {}
    /** Constructor from an ALSA event record */
    ValueEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
    ValueEvent(const snd_seq_event_type_t type, const int val);
    /** Gets the event's value */
    int getValue() const { return m_event.data.control.value; }
    /** Sets the event's value */
    void setValue( const int v ) { m_event.data.control.value = v; }
    /** Clone this object returning a pointer to the new object */
    CLONE_EVENT_DECLARATION(ValueEvent)
};

/**
 * ALSA Event representing a tempo change for an ALSA queue
 */
class DRUMSTICK_EXPORT TempoEvent : public QueueControlEvent
{
public:
    /** Default constructor */
    TempoEvent() : QueueControlEvent() {}
    /** Constructor from an ALSA event record */
    TempoEvent(snd_seq_event_t* event) : QueueControlEvent(event) {}
    TempoEvent(const int queue, const int tempo);
    /** Clone this object returning a pointer to the new object */
    CLONE_EVENT_DECLARATION(TempoEvent)
};

/**
 * ALSA Event representing a subscription between two ALSA clients and ports
 */
class DRUMSTICK_EXPORT SubscriptionEvent : public SequencerEvent
{
public:
    /** Default constructor */
    SubscriptionEvent() : SequencerEvent() {}
    /** Constructor from an ALSA event record */
    SubscriptionEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
    /** Returns true if the event was a subscribed port */
    bool subscribed() const { return (m_event.type == SND_SEQ_EVENT_PORT_SUBSCRIBED); }
    /** Returns true if the event was an unsubscribed port */
    bool unsubscribed() const { return (m_event.type == SND_SEQ_EVENT_PORT_UNSUBSCRIBED); }
    /** Gets the sender client number */
    int getSenderClient() const { return m_event.data.connect.sender.client; }
    /** Gets the sender port number */
    int getSenderPort() const { return m_event.data.connect.sender.port; }
    /** Gets the destination client number */
    int getDestClient() const { return m_event.data.connect.dest.client; }
    /** Gets the destination port number */
    int getDestPort() const { return m_event.data.connect.dest.port; }
    /** Clone this object returning a pointer to the new object */
    CLONE_EVENT_DECLARATION(SubscriptionEvent)
};

/**
 * ALSA Event representing a change on some ALSA sequencer client on the system
 */
class DRUMSTICK_EXPORT ClientEvent : public SequencerEvent
{
public:
    /** Default constructor */
    ClientEvent() : SequencerEvent() {}
    /** Constructor from an ALSA event record */
    ClientEvent(snd_seq_event_t* event) : SequencerEvent(event) {}
    int getClient() const { return m_event.data.addr.client; }
    /** Clone this object returning a pointer to the new object */
    CLONE_EVENT_DECLARATION(ClientEvent)
};

/**
 * ALSA Event representing a change on some ALSA sequencer port on the system
 */
class DRUMSTICK_EXPORT PortEvent : public ClientEvent
{
public:
    /** Default constructor */
    PortEvent() : ClientEvent() {}
    /** Constructor from an ALSA event record */
    PortEvent(snd_seq_event_t* event) : ClientEvent(event) {}
    /** Gets the port number */
    int getPort() const { return m_event.data.addr.port; }
    /** Clone this object returning a pointer to the new object */
    CLONE_EVENT_DECLARATION(PortEvent)
};

/**
 * Auxiliary class to remove events from an ALSA queue
 * @see MidiClient::removeEvents()
 */
class DRUMSTICK_EXPORT RemoveEvents
{
public:
    friend class MidiClient;

public:
    /** Default constructor */
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

/**
 * Auxiliary class to translate between raw MIDI streams and ALSA events
 */
class DRUMSTICK_EXPORT MidiCodec : public QObject
{
    Q_OBJECT
public:
    explicit MidiCodec(int bufsize, QObject* parent = 0);
    ~MidiCodec();

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

} /* namespace drumstick */

/** @} */

#endif //DRUMSTICK_ALSAEVENT_H
