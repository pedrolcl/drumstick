/*
    MIDI Sequencer C++ library
    Copyright (C) 2006-2024, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DRUMSTICK_ALSAEVENT_H
#define DRUMSTICK_ALSAEVENT_H

extern "C" {
    #include <alsa/asoundlib.h>
}

#include <QObject>
#include <QEvent>
#include "macros.h"

namespace drumstick { namespace ALSA {

Q_NAMESPACE

enum ALSASequencerEventType {
    /** system status; event data type = #snd_seq_result_t */
    System = SND_SEQ_EVENT_SYSTEM,

    /** note on and off with duration; event data type = #snd_seq_ev_note_t */
    Note = SND_SEQ_EVENT_NOTE,
    /** note on; event data type = #snd_seq_ev_note_t */
    NoteOn = SND_SEQ_EVENT_NOTEON,
    /** note off; event data type = #snd_seq_ev_note_t */
    NoteOff = SND_SEQ_EVENT_NOTEOFF,
    /** key pressure change (aftertouch); event data type = #snd_seq_ev_note_t */
    KeyPress = SND_SEQ_EVENT_KEYPRESS,

    /** controller; event data type = #snd_seq_ev_ctrl_t */
    Controller = SND_SEQ_EVENT_CONTROLLER,
    /** program change; event data type = #snd_seq_ev_ctrl_t */
    ProgramChange = SND_SEQ_EVENT_PGMCHANGE,
    /** channel pressure; event data type = #snd_seq_ev_ctrl_t */
    ChanPress = SND_SEQ_EVENT_CHANPRESS,
    /** pitchwheel; event data type = #snd_seq_ev_ctrl_t; data is from -8192 to 8191) */
    PitchBend = SND_SEQ_EVENT_PITCHBEND,
    /** 14 bit controller value; event data type = #snd_seq_ev_ctrl_t */
    Control14 = SND_SEQ_EVENT_CONTROL14,

    /** SPP with LSB and MSB values; event data type = #snd_seq_ev_ctrl_t */
    SongPos = SND_SEQ_EVENT_SONGPOS,
    /** Song Select with song ID number; event data type = #snd_seq_ev_ctrl_t */
    SongSel = SND_SEQ_EVENT_SONGSEL,
    /** midi time code quarter frame; event data type = #snd_seq_ev_ctrl_t */
    QuarterFrame = SND_SEQ_EVENT_QFRAME,
    /** SMF Time Signature event; event data type = #snd_seq_ev_ctrl_t */
    TimeSignature = SND_SEQ_EVENT_TIMESIGN,
    /** SMF Key Signature event; event data type = #snd_seq_ev_ctrl_t */
    KeySignature = SND_SEQ_EVENT_KEYSIGN,

    /** MIDI Real Time Start message; event data type = #snd_seq_ev_queue_control_t */
    RtStart = SND_SEQ_EVENT_START,
    /** MIDI Real Time Continue message; event data type = #snd_seq_ev_queue_control_t */
    RtContinue = SND_SEQ_EVENT_CONTINUE,
    /** MIDI Real Time Stop message; event data type = #snd_seq_ev_queue_control_t */
    RtStop = SND_SEQ_EVENT_STOP,
    /** (SMF) Tempo event; event data type = #snd_seq_ev_queue_control_t */
    Tempo = SND_SEQ_EVENT_TEMPO,
    /** MIDI Real Time Clock message; event data type = #snd_seq_ev_queue_control_t */
    Clock = SND_SEQ_EVENT_CLOCK,
    /** MIDI Real Time Tick message; event data type = #snd_seq_ev_queue_control_t */
    Tick = SND_SEQ_EVENT_TICK,

    /** Reset to power-on state; event data type = none */
    Reset = SND_SEQ_EVENT_RESET,
    /** Active sensing event; event data type = none */
    ActiveSensing = SND_SEQ_EVENT_SENSING,

    /** Echo-back event; event data type = any type */
    Echo = SND_SEQ_EVENT_ECHO,

    /** New client has connected; event data type = #snd_seq_addr_t */
    ClientStart = SND_SEQ_EVENT_CLIENT_START,
    /** Client has left the system; event data type = #snd_seq_addr_t */
    ClientExit = SND_SEQ_EVENT_CLIENT_EXIT,
    /** Client status/info has changed; event data type = #snd_seq_addr_t */
    ClientChange = SND_SEQ_EVENT_CLIENT_CHANGE,
    /** New port was created; event data type = #snd_seq_addr_t */
    PortStart = SND_SEQ_EVENT_PORT_START,
    /** Port was deleted from system; event data type = #snd_seq_addr_t */
    PortExit = SND_SEQ_EVENT_PORT_EXIT,
    /** Port status/info has changed; event data type = #snd_seq_addr_t */
    PortChange = SND_SEQ_EVENT_PORT_CHANGE,

    /** Ports connected; event data type = #snd_seq_connect_t */
    PortSubscribed = SND_SEQ_EVENT_PORT_SUBSCRIBED,
    /** Ports disconnected; event data type = #snd_seq_connect_t */
    PortUnsubscribed = SND_SEQ_EVENT_PORT_UNSUBSCRIBED,

    /** user-defined event; event data type = any (fixed size) */
    User0 = SND_SEQ_EVENT_USR0,
    /** user-defined event; event data type = any (fixed size) */
    User1 = SND_SEQ_EVENT_USR1,
    /** user-defined event; event data type = any (fixed size) */
    User2 = SND_SEQ_EVENT_USR2,
    /** user-defined event; event data type = any (fixed size) */
    User3 = SND_SEQ_EVENT_USR3,
    /** user-defined event; event data type = any (fixed size) */
    User4 = SND_SEQ_EVENT_USR4,
    /** user-defined event; event data type = any (fixed size) */
    User5 = SND_SEQ_EVENT_USR5,
    /** user-defined event; event data type = any (fixed size) */
    User6 = SND_SEQ_EVENT_USR6,
    /** user-defined event; event data type = any (fixed size) */
    User7 = SND_SEQ_EVENT_USR7,
    /** user-defined event; event data type = any (fixed size) */
    User8 = SND_SEQ_EVENT_USR8,
    /** user-defined event; event data type = any (fixed size) */
    User9 = SND_SEQ_EVENT_USR9,

    /** system exclusive data (variable length);  event data type = #snd_seq_ev_ext_t */
    Sysex = SND_SEQ_EVENT_SYSEX,

    /** reserved for user apps;  event data type = #snd_seq_ev_ext_t */
    Var0 = SND_SEQ_EVENT_USR_VAR0,
    /** reserved for user apps; event data type = #snd_seq_ev_ext_t */
    Var1 = SND_SEQ_EVENT_USR_VAR1,
    /** reserved for user apps; event data type = #snd_seq_ev_ext_t */
    Var2 = SND_SEQ_EVENT_USR_VAR2,
    /** reserved for user apps; event data type = #snd_seq_ev_ext_t */
    Var3 = SND_SEQ_EVENT_USR_VAR3,
    /** reserved for user apps; event data type = #snd_seq_ev_ext_t */
    Var4 = SND_SEQ_EVENT_USR_VAR4
};

Q_ENUM_NS(ALSASequencerEventType);

/**
 * @file alsaevent.h
 * Classes managing ALSA Sequencer events.
 *
 * @addtogroup ALSAEvent ALSA Sequencer Events
 * @{
 */

#if defined(DRUMSTICK_STATIC)
#define DRUMSTICK_ALSA_EXPORT
#else
#if defined(drumstick_alsa_EXPORTS)
#define DRUMSTICK_ALSA_EXPORT Q_DECL_EXPORT
#else
#define DRUMSTICK_ALSA_EXPORT Q_DECL_IMPORT
#endif
#endif

/**
 * 8-bit unsigned number to be used as a MIDI message parameter
 */
typedef quint8 MidiByte;

/**
 * Constant SequencerEventType is the QEvent::type() of any SequencerEvent
 * object to be used to check the argument in QObject::customEvent().
 */
const QEvent::Type SequencerEventType = QEvent::Type(QEvent::User + 4154); // :-)

/**
 * Base class for the event's hierarchy
 *
 * All event classes share this base class. It provides several common
 * properties and methods.
 */
class DRUMSTICK_ALSA_EXPORT SequencerEvent : public QEvent
{
public:
    SequencerEvent();
    SequencerEvent(const SequencerEvent& other);
    explicit SequencerEvent(const snd_seq_event_t* event);

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
    virtual SequencerEvent* clone() const;

protected:
    Q_DECL_DEPRECATED void free();

    /**
     * ALSA sequencer event record.
     * @see https://www.alsa-project.org/alsa-doc/alsa-lib/structsnd__seq__event.html
     */
    snd_seq_event_t m_event;
};

/**
 * Base class for the events having a Channel property
 */
class DRUMSTICK_ALSA_EXPORT ChannelEvent : public SequencerEvent
{
public:
    /** Default constructor */
    ChannelEvent() : SequencerEvent() {}
    /**
     * Constructor from an ALSA event record
     * @param event an ALSA event record
     */
    explicit ChannelEvent(const snd_seq_event_t* event) : SequencerEvent(event) {}
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

    virtual ChannelEvent* clone() const override;
};

/**
 * Base class for the events having Key and Velocity properties.
 */
class DRUMSTICK_ALSA_EXPORT KeyEvent : public ChannelEvent
{
public:
    /** Default constructor */
    KeyEvent() : ChannelEvent() {}
    /**
     * Constructor from an ALSA event record
     * @param event an ALSA event record
     */
    explicit KeyEvent(const snd_seq_event_t* event) : ChannelEvent(event) {}
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

    virtual KeyEvent* clone() const override;
};

/**
 * Class representing a note event with duration
 *
 * Note events are converted into two MIDI events, a note-on and a note-off
 * over the wire.
 */
class DRUMSTICK_ALSA_EXPORT NoteEvent : public KeyEvent
{
public:
    /** Default constructor */
    NoteEvent() : KeyEvent() { m_event.type = SND_SEQ_EVENT_NOTE; }
    /**
     * Constructor from an ALSA event record
     * @param event an ALSA event record
     */
    explicit NoteEvent(const snd_seq_event_t* event) : KeyEvent(event) {}
    /**
     * Constructor
     */
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

    virtual NoteEvent* clone() const override;
};

/**
 * Event representing a note-on MIDI event
 */
class DRUMSTICK_ALSA_EXPORT NoteOnEvent : public KeyEvent
{
public:
    /** Default constructor */
    NoteOnEvent() : KeyEvent() { m_event.type = SND_SEQ_EVENT_NOTEON; }
    /**
     * Constructor from an ALSA event record
     * @param event an ALSA event record
     */
    explicit NoteOnEvent(const snd_seq_event_t* event) : KeyEvent(event) {}
    /**
     * Constructor
     */
    NoteOnEvent(const int ch, const int key, const int vel);
    virtual NoteOnEvent* clone() const override;
};

/**
 * Event representing a note-off MIDI event
 */
class DRUMSTICK_ALSA_EXPORT NoteOffEvent : public KeyEvent
{
public:
    /** Default constructor */
    NoteOffEvent() : KeyEvent() { m_event.type = SND_SEQ_EVENT_NOTEOFF; }
    /**
     * Constructor from an ALSA event record
     * @param event an ALSA event record
     */
    explicit NoteOffEvent(const snd_seq_event_t* event) : KeyEvent(event) {}
    /**
     * Constructor
     */
    NoteOffEvent(const int ch, const int key, const int vel);
    virtual NoteOffEvent* clone() const override;
};

/**
 * Event representing a MIDI key pressure, or polyphonic after-touch event
 */
class DRUMSTICK_ALSA_EXPORT KeyPressEvent : public KeyEvent
{
public:
    /** Default constructor */
    KeyPressEvent() : KeyEvent() { m_event.type = SND_SEQ_EVENT_KEYPRESS; }
    /**
     * Constructor from an ALSA event record
     * @param event an ALSA event record
     */
    explicit KeyPressEvent(const snd_seq_event_t* event) : KeyEvent(event) {}
    /**
     * Constructor
     */
    KeyPressEvent(const int ch, const int key, const int vel);
    virtual KeyPressEvent* clone() const override;
};

/**
 * Event representing a MIDI control change event
 */
class DRUMSTICK_ALSA_EXPORT ControllerEvent : public ChannelEvent
{
public:
    /** Default constructor */
    ControllerEvent() : ChannelEvent() {}
    /**
     * Constructor from an ALSA event record
     * @param event an ALSA event record
     */
    explicit ControllerEvent(const snd_seq_event_t* event) : ChannelEvent(event) {}
    /**
     * Constructor
     */
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
    virtual ControllerEvent* clone() const override;
};

/**
 * Event representing a MIDI program change event
 */
class DRUMSTICK_ALSA_EXPORT ProgramChangeEvent : public ChannelEvent
{
public:
    /** Default constructor */
    ProgramChangeEvent() : ChannelEvent() { m_event.type = SND_SEQ_EVENT_PGMCHANGE; }
    /**
     * Constructor from an ALSA event record
     * @param event an ALSA event record
     */
    explicit ProgramChangeEvent(const snd_seq_event_t* event) : ChannelEvent(event) {}
    /**
     * Constructor
     */
    ProgramChangeEvent(const int ch, const int val);
    /**
     * Gets the MIDI program number
     * @return the MIDI program number
     */
    int getValue() const { return m_event.data.control.value; }
    /**
     * Sets the MIDI program number
     * @param v the MIDI program number
     */
    void setValue( const int v ) { m_event.data.control.value = v; }
    virtual ProgramChangeEvent* clone() const override;
};

/**
 * Event representing a MIDI bender, or pitch wheel event
 */
class DRUMSTICK_ALSA_EXPORT PitchBendEvent : public ChannelEvent
{
public:
    /** Default constructor */
    PitchBendEvent() : ChannelEvent() { m_event.type = SND_SEQ_EVENT_PITCHBEND; }
    /**
     * Constructor from an ALSA event record
     * @param event an ALSA event record
     */
    explicit PitchBendEvent(const snd_seq_event_t* event) : ChannelEvent(event) {}
    /**
     * Constructor
     */
    PitchBendEvent(const int ch, const int val);
    /**
     * Gets the MIDI pitch bend value, zero centered from -8192 to 8191
     * @return the MIDI pitch bend value
     */
    int getValue() const { return m_event.data.control.value; }
    /**
     * Sets the MIDI pitch bend value, zero centered from -8192 to 8191
     * @param v the MIDI pitch bend value
     */
    void setValue( const int v ) { m_event.data.control.value = v; }
    virtual PitchBendEvent* clone() const override;
};

/**
 * Event representing a MIDI channel pressure or after-touch event
 */
class DRUMSTICK_ALSA_EXPORT ChanPressEvent : public ChannelEvent
{
public:
    /** Default constructor */
    ChanPressEvent() : ChannelEvent() { m_event.type = SND_SEQ_EVENT_CHANPRESS; }
    /**
     * Constructor from an ALSA event record
     * @param event an ALSA event record
     */
    explicit ChanPressEvent( const snd_seq_event_t* event ) : ChannelEvent(event) {}
    /**
     * Constructor
     */
    ChanPressEvent( const int ch, const int val );
    /**
     * Gets the channel aftertouch value
     * @return the channel aftertouch value
     */
    int getValue() const { return m_event.data.control.value; }
    /**
     * Sets the channel aftertouch value
     * @param v the channel aftertouch value
     */
    void setValue( const int v ) { m_event.data.control.value = v; }
    virtual ChanPressEvent* clone() const override;
};

/**
 * Base class for variable length events
 */
class DRUMSTICK_ALSA_EXPORT VariableEvent : public SequencerEvent
{
public:
    VariableEvent();
    explicit VariableEvent(const snd_seq_event_t* event);
    explicit VariableEvent(const QByteArray& data);
    VariableEvent(const VariableEvent& other);
    VariableEvent(const unsigned int datalen, char* dataptr);
    VariableEvent& operator=(const VariableEvent& other);
    /**
     * Gets the data length
     * @return the data length
     */
    unsigned int getLength() const { return m_event.data.ext.len; }
    /**
     * Gets the data pointer
     * @return the data pointer
     */
    const char* getData() const { return static_cast<const char*>(m_event.data.ext.ptr); }
    virtual VariableEvent* clone() const override;
protected:
    QByteArray m_data;
};

/**
 * Event representing a MIDI system exclusive event
 */
class DRUMSTICK_ALSA_EXPORT SysExEvent : public VariableEvent
{
public:
    SysExEvent();
    explicit SysExEvent(const snd_seq_event_t* event);
    explicit SysExEvent(const QByteArray& data);
    SysExEvent(const SysExEvent& other);
    SysExEvent(const unsigned int datalen, char* dataptr);
    SysExEvent &operator=(const SysExEvent &other);
    virtual SysExEvent* clone() const override;
};

/**
 * Event representing a SMF text event
 *
 * This event type is not intended to be transmitted over the wire to an
 * external device, but it is useful for sequencer programs or MIDI applications
 */
class DRUMSTICK_ALSA_EXPORT TextEvent : public VariableEvent
{
public:
    TextEvent();
    explicit TextEvent(const snd_seq_event_t* event);
    explicit TextEvent(const QString& text, const int textType = 1);
    TextEvent(const TextEvent& other);
    TextEvent(const unsigned int datalen, char* dataptr);
    TextEvent &operator=(const TextEvent &other);
    QString getText() const;
    int getTextType() const;
    virtual TextEvent* clone() const override;
protected:
    int m_textType;
};

/**
 * Generic event
 */
class DRUMSTICK_ALSA_EXPORT SystemEvent : public SequencerEvent
{
public:
    /** Default constructor */
    SystemEvent() : SequencerEvent() {}
    /**
     * Constructor from an ALSA event record
     * @param event an ALSA event record
     */
    explicit SystemEvent(const snd_seq_event_t* event) : SequencerEvent(event) {}
    explicit SystemEvent(const snd_seq_event_type_t type);
    virtual SystemEvent* clone() const override;
};

/**
 * ALSA Event representing a queue control command
 *
 * This event is used to schedule changes to the ALSA queues
 */
class DRUMSTICK_ALSA_EXPORT QueueControlEvent : public SequencerEvent
{
public:
    /** Default constructor */
    QueueControlEvent() : SequencerEvent() {}
    /**
     * Constructor from an ALSA event record
     * @param event an ALSA event record
     */
    explicit QueueControlEvent(const snd_seq_event_t* event) : SequencerEvent(event) {}
    QueueControlEvent(const snd_seq_event_type_t type, const int queue, const int value);
    /**
     * Gets the queue number
     * @return the queue number
     */
    int getQueue() const { return m_event.data.queue.queue; }
    /**
     * Sets the queue number
     * @param q the queue number
     */
    void setQueue(const uchar q) { m_event.data.queue.queue = q; }
    /**
     * Gets the event's value
     * @return the event's value
     */
    int getValue() const { return m_event.data.queue.param.value; }
    /**
     * Sets the event's value
     * @param val the event's value
     */
    void setValue(const int val) { m_event.data.queue.param.value = val; }
    /**
     * Gets the queue position
     * @return the queue position
     */
    uint getPosition() const { return m_event.data.queue.param.position; }
    /**
     * Sets the queue position
     * @param pos the queue position
     */
    void setPosition(const uint pos) { m_event.data.queue.param.position = pos; }
    /**
     * Gets the musical time in ticks
     * @return the musical time in ticks
     */
    snd_seq_tick_time_t getTickTime() const { return m_event.data.queue.param.time.tick; }
    /**
     * Sets the musical time in ticks
     * @param t the musical time in ticks
     */
    void setTickTime(const snd_seq_tick_time_t t) { m_event.data.queue.param.time.tick = t; }
    /**
     * Gets the skew base
     * @return the skew base
     */
    uint getSkewBase() const { return m_event.data.queue.param.skew.base;  }
    /**
     * Sets the skew base, should be 65536
     * @param base the skew base, should be 65536
     */
    void setSkewBase(const uint base) { m_event.data.queue.param.skew.base = base; }
    /**
     * Gets the skew value
     * @return the skew value
     */
    uint getSkewValue() const { return m_event.data.queue.param.skew.value;  }
    /**
     * Sets the skew value
     * @param val the skew value
     */
    void setSkewValue(const uint val) {m_event.data.queue.param.skew.value = val; }
    virtual QueueControlEvent* clone() const override;
};

/**
 * Generic event having a value property
 */
class DRUMSTICK_ALSA_EXPORT ValueEvent : public SequencerEvent
{
public:
    /** Default constructor */
    ValueEvent() : SequencerEvent() {}
    /**
     * Constructor from an ALSA event record
     * @param event an ALSA event record
     */
    explicit ValueEvent(const snd_seq_event_t* event) : SequencerEvent(event) {}
    ValueEvent(const snd_seq_event_type_t type, const int val);
    /**
     * Gets the event's value
     * @return the event's value
     */
    int getValue() const { return m_event.data.control.value; }
    /**
     * Sets the event's value
     * @param v the event's value
     */
    void setValue( const int v ) { m_event.data.control.value = v; }
    virtual ValueEvent* clone() const override;
};

/**
 * ALSA Event representing a tempo change for an ALSA queue
 */
class DRUMSTICK_ALSA_EXPORT TempoEvent : public QueueControlEvent
{
public:
    /** Default constructor */
    TempoEvent() : QueueControlEvent() {}
    /**
     * Constructor from an ALSA event record
     * @param event an ALSA event record
     */
    explicit TempoEvent(const snd_seq_event_t* event) : QueueControlEvent(event) {}
    TempoEvent(const int queue, const int tempo);
    virtual TempoEvent* clone() const override;
};

/**
 * ALSA Event representing a subscription between two ALSA clients and ports
 */
class DRUMSTICK_ALSA_EXPORT SubscriptionEvent : public SequencerEvent
{
public:
    /** Default constructor */
    SubscriptionEvent() : SequencerEvent() {}
    /**
     * Constructor from an ALSA event record
     * @param event an ALSA event record
     */
    explicit SubscriptionEvent(const snd_seq_event_t* event) : SequencerEvent(event) {}
    /**
     * Returns true if the event was a subscribed port
     * @return whether the event was a subscribed port
     */
    bool subscribed() const { return (m_event.type == SND_SEQ_EVENT_PORT_SUBSCRIBED); }
    /**
     * Returns true if the event was an unsubscribed port
     * @return whether the event was an unsubscribed port
     */
    bool unsubscribed() const { return (m_event.type == SND_SEQ_EVENT_PORT_UNSUBSCRIBED); }
    /**
     * Gets the sender client number
     * @return the sender client number
     */
    int getSenderClient() const { return m_event.data.connect.sender.client; }
    /**
     * Gets the sender port number
     * @return the sender port number
     */
    int getSenderPort() const { return m_event.data.connect.sender.port; }
    /**
     * Gets the destination client number
     * @return the destination client number
     */
    int getDestClient() const { return m_event.data.connect.dest.client; }
    /**
     * Gets the destination port number
     * @return the destination port number
     */
    int getDestPort() const { return m_event.data.connect.dest.port; }
    virtual SubscriptionEvent* clone() const override;
};

/**
 * ALSA Event representing a change on some ALSA sequencer client on the system
 */
class DRUMSTICK_ALSA_EXPORT ClientEvent : public SequencerEvent
{
public:
    /** Default constructor */
    ClientEvent() : SequencerEvent() {}
    /**
     * Constructor from an ALSA event record
     * @param event an ALSA event record
     */
    explicit ClientEvent(const snd_seq_event_t* event) : SequencerEvent(event) {}
    /**
     * Gets the client number
     * @return the client number
     */
    int getClient() const { return m_event.data.addr.client; }
    virtual ClientEvent* clone() const override;
};

/**
 * ALSA Event representing a change on some ALSA sequencer port on the system
 */
class DRUMSTICK_ALSA_EXPORT PortEvent : public ClientEvent
{
public:
    /** Default constructor */
    PortEvent() : ClientEvent() {}
    /**
     * Constructor from an ALSA event record
     * @param event an ALSA event record
     */
    explicit PortEvent(const snd_seq_event_t* event) : ClientEvent(event) {}
    /**
     * Gets the port number
     * @return the port number
     */
    int getPort() const { return m_event.data.addr.port; }
    virtual PortEvent* clone() const override;
};

/**
 * Auxiliary class to remove events from an ALSA queue
 * @see MidiClient::removeEvents()
 */
class DRUMSTICK_ALSA_EXPORT RemoveEvents
{
public:
    friend class MidiClient;

public:
    RemoveEvents();
    RemoveEvents(const RemoveEvents& other);
    explicit RemoveEvents(snd_seq_remove_events_t* other);
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
class DRUMSTICK_ALSA_EXPORT MidiCodec : public QObject
{
    Q_OBJECT
public:
    explicit MidiCodec(int bufsize, QObject* parent = nullptr);
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

QDebug operator<<(QDebug d, const SequencerEvent &event);
QDebug operator<<(QDebug d, const SequencerEvent *event);

/** @} */

}} /* namespace drumstick::ALSA */

Q_DECLARE_METATYPE(drumstick::ALSA::SequencerEvent)
Q_DECLARE_METATYPE(drumstick::ALSA::SequencerEvent*)

#endif //DRUMSTICK_ALSAEVENT_H
