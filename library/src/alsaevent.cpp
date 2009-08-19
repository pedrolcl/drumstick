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

#include "alsaevent.h"

/**
 * @class QEvent
 * The QEvent class is the base class of all event classes.
 * @see http://doc.trolltech.com/qevent.html
 */

namespace aseqmm {

/**
 * Default constructor.
 */
SequencerEvent::SequencerEvent() : QEvent(SequencerEventType)
{
    snd_seq_ev_clear( &m_event );
}

/**
 * Constructor from an ALSA event record
 * @param event ALSA event record
 */
SequencerEvent::SequencerEvent(snd_seq_event_t* event) : QEvent(SequencerEventType)
{
    snd_seq_ev_clear( &m_event );
    m_event = *event;
}

/**
 * Copy constructor
 * @param other A SequencerEvent object reference
 */
SequencerEvent::SequencerEvent(const SequencerEvent& other) : QEvent(SequencerEventType)
{
    snd_seq_ev_clear( &m_event );
    m_event = other.m_event;
}

/**
 * Assignment operator
 * @param other A SequencerEvent object reference
 * @return This object
 */
SequencerEvent&
SequencerEvent::operator=(const SequencerEvent& other)
{
    m_event = other.m_event;
    return *this;
}

/**
 * Checks if the event's type is a subscription.
 * @param event A SequencerEvent object pointer
 * @return True if the event has a subscribe/unsubscribe type.
 */
bool isSubscription(const SequencerEvent* event)
{
    snd_seq_event_type_t te = event->getSequencerType();
    return ( te == SND_SEQ_EVENT_PORT_SUBSCRIBED ||
             te == SND_SEQ_EVENT_PORT_UNSUBSCRIBED );
}

/**
 * Checks if the event's type is of type port.
 * @param event A SequencerEvent object pointer
 * @return True if the event has a port start/exit/change type.
 */
bool isPort(const SequencerEvent* event)
{
    snd_seq_event_type_t te = event->getSequencerType();
    return ( te == SND_SEQ_EVENT_PORT_START ||
             te == SND_SEQ_EVENT_PORT_EXIT ||
             te == SND_SEQ_EVENT_PORT_CHANGE );
}

/**
 * Checks if the event's type is of type client.
 * @param event A SequencerEvent object pointer
 * @return True if the event has a client start/exit/change type.
 */
bool isClient(const SequencerEvent* event)
{
    snd_seq_event_type_t te = event->getSequencerType();
    return ( te == SND_SEQ_EVENT_CLIENT_START ||
             te == SND_SEQ_EVENT_CLIENT_EXIT ||
             te == SND_SEQ_EVENT_CLIENT_CHANGE );
}

/**
 * Checks if the event's type is of type connection change.
 * @param event A SequencerEvent object pointer
 * @return True if the event has a client/port/subscription type.
 */
bool isConnectionChange(const SequencerEvent* event)
{
    snd_seq_event_type_t te = event->getSequencerType();
    return ( te == SND_SEQ_EVENT_PORT_START ||
             te == SND_SEQ_EVENT_PORT_EXIT ||
             te == SND_SEQ_EVENT_PORT_CHANGE ||
             te == SND_SEQ_EVENT_CLIENT_START ||
             te == SND_SEQ_EVENT_CLIENT_EXIT ||
             te == SND_SEQ_EVENT_CLIENT_CHANGE ||
             te == SND_SEQ_EVENT_PORT_SUBSCRIBED ||
             te == SND_SEQ_EVENT_PORT_UNSUBSCRIBED );
}

/**
 * Sets the event's ALSA sequencer type
 * @param eventType The ALSA sequencer type
 */
void SequencerEvent::setSequencerType(const snd_seq_event_type_t eventType)
{
    m_event.type = eventType;
}

/**
 * Sets the client:port destination of the event.
 * @param client The destination's client ID
 * @param port The destination port ID
 * @see setSubscribers()
 */
void SequencerEvent::setDestination(const unsigned char client, const unsigned char port)
{
    snd_seq_ev_set_dest(&m_event, client, port);
}

/**
 * Sets the event's source port ID
 * @param port The source port ID
 * @see getSourceClient(), getSourcePort()
 */
void SequencerEvent::setSource(const unsigned char port)
{
    snd_seq_ev_set_source(&m_event, port);
}

/**
 * Sets the event's destination to be all the subscribers of the source port.
 */
void SequencerEvent::setSubscribers()
{
    snd_seq_ev_set_subs(&m_event);
}

/**
 * Sets the event's destination to be all queues/clients/ports/channels.
 */
void SequencerEvent::setBroadcast()
{
    snd_seq_ev_set_broadcast(&m_event);
}

/**
 * Sets the event to be immediately delivered, not queued/scheduled.
 * @see scheduleTick(), scheduleReal()
 */
void SequencerEvent::setDirect()
{
    snd_seq_ev_set_direct(&m_event);
}

/**
 * Sets the event to be scheduled in musical time (ticks) units.
 * @param queue The queue number to be used.
 * @param tick The time in ticks.
 * @param relative Use relative (to the current) time instead of absolute time.
 */
void SequencerEvent::scheduleTick(int queue, int tick, bool relative)
{
    snd_seq_ev_schedule_tick(&m_event, queue, relative, tick);
}

/**
 * Sets the event to be scheduled in real (clock) time units.
 * @param queue The queue number to be used.
 * @param secs The time in whole seconds.
 * @param nanos The nanoseconds to be added.
 * @param relative Use relative (to the current) time instead of absolute time.
 */
void SequencerEvent::scheduleReal(int queue, ulong secs, ulong nanos, bool relative)
{
    snd_seq_real_time_t rtime;
    rtime.tv_sec = secs;
    rtime.tv_nsec = nanos;
    snd_seq_ev_schedule_real(&m_event, queue, relative, &rtime);
}

/**
 * Sets the priority of the event. This is used in case of several events share
 * the same scheduling time.
 *
 * @param high Mark the event as a high priority one.
 */
void SequencerEvent::setPriority(const bool high)
{
    snd_seq_ev_set_priority(&m_event, high);
}

/**
 * Sets the event's tag. This attribute is any arbitrary number, not used by
 * the ALSA library. Range limited to 0 thru 255.
 * @param aTag A tag number.
 */
void SequencerEvent::setTag(const unsigned char aTag)
{
#if SND_LIB_SUBMINOR > 8
    snd_seq_ev_set_tag(&m_event, aTag);
#else
    m_event.tag = aTag;
#endif
}

/**
 * Gets an event's raw 32 bits parameter.
 * @param n The parameter index, between 0 and 2.
 * @return The parameter's value.
 * @see setRaw32()
 */
unsigned int SequencerEvent::getRaw32(const unsigned int n) const
{
    if (n < 3) return m_event.data.raw32.d[n];
    return 0;
}

/**
 * Sets an event's raw 32 bits parameter.
 * @param n The parameter index, between 0 and 2.
 * @param value The parameter's value.
 */
void SequencerEvent::setRaw32(const unsigned int n, const unsigned int value)
{
    if (n < 3) m_event.data.raw32.d[n] = value;
}

/**
 * Gets an event's raw 8 bits parameter.
 * @param n The parameter index, between 0 and 11.
 * @return The parameter's value.
 * @see setRaw8()
 */
unsigned char SequencerEvent::getRaw8(const unsigned int n) const
{
    if (n < 12) return m_event.data.raw8.d[n];
    return 0;
}

/**
 * Sets an event's raw 8 bits parameter.
 * @param n The parameter index, between 0 and 11.
 * @param value The parameter's value.
 */
void SequencerEvent::setRaw8(const unsigned int n, const unsigned char value)
{
    if (n < 12) m_event.data.raw8.d[n] = value;
}

/**
 * Releases the event record (deprecated).
 * @deprecated
 */
void SequencerEvent::free()
{
    snd_seq_free_event(&m_event);
}

/**
 * Gets the encoded length of the event record.
 * @return The encoded length.
 */
int SequencerEvent::getEncodedLength()
{
    return snd_seq_event_length(&m_event);
}

/**
 * Constructor using proper attribute values.
 * @param ch MIDI Channel.
 * @param key MIDI note.
 * @param vel Note velocity.
 * @param dur Note duration.
 */
NoteEvent::NoteEvent(int ch, int key, int vel, int dur) : KeyEvent()
{
    snd_seq_ev_set_note(&m_event, ch, key, vel, dur);
}

/**
 * Constructor using proper attribute values.
 * @param ch MIDI Channel.
 * @param key MIDI note.
 * @param vel Note velocity.
 */
NoteOnEvent::NoteOnEvent(int ch, int key, int vel) : KeyEvent()
{
    snd_seq_ev_set_noteon(&m_event, ch, key, vel);
}

/**
 * Constructor using proper attribute values.
 * @param ch MIDI Channel.
 * @param key MIDI note.
 * @param vel Note velocity.
 */
NoteOffEvent::NoteOffEvent(int ch, int key, int vel) : KeyEvent()
{
    snd_seq_ev_set_noteoff(&m_event, ch, key, vel);
}

/**
 * Constructor using proper attribute values.
 * @param ch MIDI Channel.
 * @param key MIDI note.
 * @param vel Note velocity.
 */
KeyPressEvent::KeyPressEvent(int ch, int key, int vel) : KeyEvent()
{
    snd_seq_ev_set_keypress(&m_event, ch, key, vel);
}

/**
 * Constructor using proper attribute values.
 * @param ch MIDI Channel.
 * @param cc MIDI Controller number.
 * @param val Controller value.
 */
ControllerEvent::ControllerEvent(int ch, int cc, int val) : ChannelEvent()
{
    snd_seq_ev_set_controller(&m_event, ch, cc, val);
}

/**
 * Constructor using proper attribute values.
 * @param ch MIDI Channel.
 * @param val MIDI Program number.
 */
ProgramChangeEvent::ProgramChangeEvent(int ch, int val) : ChannelEvent()
{
    snd_seq_ev_set_pgmchange(&m_event, ch, val);
}

/**
 * Constructor using proper attribute values.
 * @param ch MIDI Channel.
 * @param val Pitch Bend value. Zero centered from -8192 to 8191.
 */
PitchBendEvent::PitchBendEvent(int ch, int val) : ChannelEvent()
{
    snd_seq_ev_set_pitchbend(&m_event, ch, val);
}

/**
 * Constructor using proper attribute values.
 * @param ch MDII Channel.
 * @param val Aftertouch value.
 */
ChanPressEvent::ChanPressEvent(int ch, int val) : ChannelEvent()
{
    snd_seq_ev_set_chanpress(&m_event, ch, val);
}

/**
 * Default constructor.
 */
VariableEvent::VariableEvent()
    : SequencerEvent()
{
    m_data.clear();
    snd_seq_ev_set_variable ( &m_event, m_data.size(), m_data.data() );
}

/**
 * Constructor from an ALSA event record.
 * @param event ALSA event record.
 */
VariableEvent::VariableEvent(snd_seq_event_t* event)
    : SequencerEvent(event)
{
    m_data = QByteArray((char *) event->data.ext.ptr,
                        event->data.ext.len);
    snd_seq_ev_set_variable ( &m_event, m_data.size(), m_data.data() );
}

/**
 * Constructor from an arbitrary data array.
 * @param data A data byte array.
 */
VariableEvent::VariableEvent(const QByteArray& data)
    : SequencerEvent()
{
    m_data = data;
    snd_seq_ev_set_variable ( &m_event, m_data.size(), m_data.data() );
}

/**
 * Copy constructor.
 * @param other Another VariableEvent instance.
s */
VariableEvent::VariableEvent(const VariableEvent& other)
    : SequencerEvent()
{
    m_data = other.m_data;
    snd_seq_ev_set_variable ( &m_event, m_data.size(), m_data.data() );
}

/**
 * Constructor from a data pointer.
 * @param datalen Length of the data.
 * @param dataptr Pointer the data.
 */
VariableEvent::VariableEvent(const unsigned int datalen, char* dataptr)
    : SequencerEvent()
{
    m_data = QByteArray(dataptr, datalen);
    snd_seq_ev_set_variable( &m_event, m_data.size(), m_data.data() );
}

/**
 * Assignment operator.
 * @param other Another VariableEvent object reference
 * @return Pointer to this object
 */
VariableEvent& VariableEvent::operator=(const VariableEvent& other)
{
    m_event = other.m_event;
    m_data = other.m_data;
    snd_seq_ev_set_variable ( &m_event, m_data.size(), m_data.data() );
    return *this;
}

/**
 * Default constructor.
 */
SysExEvent::SysExEvent()
    : VariableEvent()
{
    snd_seq_ev_set_sysex( &m_event, m_data.size(), m_data.data() );
}

/**
 * Constructor from an ALSA event record.
 * @param event ALSA event record.
 */
SysExEvent::SysExEvent(snd_seq_event_t* event)
    : VariableEvent(event)
{
    snd_seq_ev_set_sysex( &m_event, m_data.size(), m_data.data() );
}

/**
 * Constructor from a data array.
 * @param data A data byte array.
 */
SysExEvent::SysExEvent(const QByteArray& data)
    : VariableEvent(data)
{
    snd_seq_ev_set_sysex( &m_event, m_data.size(), m_data.data() );
}

/**
 * Copy constructor.
 * @param other Another SysExEvent object reference.
 */
SysExEvent::SysExEvent(const SysExEvent& other)
    : VariableEvent(other)
{
    snd_seq_ev_set_sysex( &m_event, m_data.size(), m_data.data() );
}

/**
 * Constructor taking a data pointer and length
 * @param datalen Data length
 * @param dataptr Data pointer
 */
SysExEvent::SysExEvent(const unsigned int datalen, char* dataptr)
    : VariableEvent( datalen, dataptr )
{
    snd_seq_ev_set_sysex( &m_event, m_data.size(), m_data.data() );
}

/**
 * Default constructor
 */
TextEvent::TextEvent()
    : VariableEvent(), m_textType(1)
{
    setSequencerType(SND_SEQ_EVENT_USR_VAR0);
}

/**
 * Constructor from an ALSA sequencer record.
 * @param event ALSA sequencer record.
 */
TextEvent::TextEvent(snd_seq_event_t* event)
    : VariableEvent(event), m_textType(1)
{
    setSequencerType(SND_SEQ_EVENT_USR_VAR0);
}

/**
 * Constructor from a given string
 * @param text The event's text
 * @param textType The SMF text type
 */
TextEvent::TextEvent(const QString& text, const int textType)
    : VariableEvent(text.toAscii()), m_textType(textType)
{
    setSequencerType(SND_SEQ_EVENT_USR_VAR0);
}

/**
 * Copy constructor
 * @param other An existing TextEvent object reference
 */
TextEvent::TextEvent(const TextEvent& other)
    : VariableEvent(other)
{
    setSequencerType(SND_SEQ_EVENT_USR_VAR0);
    m_textType = other.getTextType();
}

/**
 * Constructor from a data pointer and length
 * @param datalen Data length
 * @param dataptr Data pointer
 */
TextEvent::TextEvent(const unsigned int datalen, char* dataptr)
    : VariableEvent(datalen, dataptr), m_textType(1)
{
    setSequencerType(SND_SEQ_EVENT_USR_VAR0);
}

/**
 * Gets the event's text content.
 * @return The text content.
 */
QString TextEvent::getText() const
{
    return QString::fromAscii(m_data.data(), m_data.size());
}

/**
 * Gets the event's SMF text type.
 * @return The SMF text type.
 */
int TextEvent::getTextType() const
{
    return m_textType;
}

/**
 * Constructor
 * @param statusByte The event's status byte
 */
SystemEvent::SystemEvent(int statusByte) : SequencerEvent()
{
    snd_seq_ev_set_fixed(&m_event);
    setSequencerType(statusByte);
}

/**
 * Constructor
 * @param type Event type
 * @param queue Queue number
 * @param value Value
 */
QueueControlEvent::QueueControlEvent(int type, int queue, int value)
    : SequencerEvent()
{
    snd_seq_ev_set_queue_control(&m_event, type, queue, value);
}

/**
 * Constructor
 * @param statusByte The event's status byte
 * @param val Value
 */
ValueEvent::ValueEvent(int statusByte, int val) : SequencerEvent()
{
    snd_seq_ev_set_fixed(&m_event);
    setSequencerType(statusByte);
    setValue(val);
}

/**
 * Constructor
 * @param queue Queue number.
 * @param tempo Tempo value in microseconds per quarter note.
 */
TempoEvent::TempoEvent(int queue, int tempo) : QueueControlEvent()
{
    snd_seq_ev_set_queue_tempo(&m_event, queue, tempo);
}

/****************
 * RemoveEvents *
 ****************/

RemoveEvents::RemoveEvents()
{
    snd_seq_remove_events_malloc(&m_Info);
}

RemoveEvents::RemoveEvents(const RemoveEvents& other)
{
    snd_seq_remove_events_malloc(&m_Info);
    snd_seq_remove_events_copy(m_Info, other.m_Info);
}

RemoveEvents::RemoveEvents(snd_seq_remove_events_t* other)
{
    snd_seq_remove_events_malloc(&m_Info);
    snd_seq_remove_events_copy(m_Info, other);
}

RemoveEvents::~RemoveEvents()
{
    snd_seq_remove_events_free(m_Info);
}

RemoveEvents*
RemoveEvents::clone()
{
    return new RemoveEvents(m_Info);
}

RemoveEvents&
RemoveEvents::operator=(const RemoveEvents& other)
{
    snd_seq_remove_events_copy(m_Info, other.m_Info);
    return *this;
}

int
RemoveEvents::getSizeOfInfo() const
{
    return snd_seq_remove_events_sizeof();
}

int
RemoveEvents::getChannel()
{
    return snd_seq_remove_events_get_channel(m_Info);
}

unsigned int
RemoveEvents::getCondition()
{
    return snd_seq_remove_events_get_condition(m_Info);
}

const snd_seq_addr_t*
RemoveEvents::getDest()
{
    return snd_seq_remove_events_get_dest(m_Info);
}

int
RemoveEvents::getEventType()
{
    return snd_seq_remove_events_get_event_type(m_Info);
}

int
RemoveEvents::getQueue()
{
    return snd_seq_remove_events_get_queue(m_Info);
}

int
RemoveEvents::getTag()
{
    return snd_seq_remove_events_get_tag(m_Info);
}

const snd_seq_timestamp_t*
RemoveEvents::getTime()
{
    return snd_seq_remove_events_get_time(m_Info);
}

void
RemoveEvents::setChannel(int chan)
{
    snd_seq_remove_events_set_channel(m_Info, chan);
}

void
RemoveEvents::setCondition(unsigned int cond)
{
    snd_seq_remove_events_set_condition(m_Info, cond);
}

void
RemoveEvents::setDest(const snd_seq_addr_t* dest)
{
    snd_seq_remove_events_set_dest(m_Info, dest);
}

void
RemoveEvents::setEventType(int type)
{
    snd_seq_remove_events_set_event_type(m_Info, type);
}

void
RemoveEvents::setQueue(int queue)
{
    snd_seq_remove_events_set_queue(m_Info, queue);
}

void
RemoveEvents::setTag(int tag)
{
    snd_seq_remove_events_set_tag(m_Info, tag);
}

void
RemoveEvents::setTime(const snd_seq_timestamp_t* time)
{
    snd_seq_remove_events_set_time(m_Info, time);
}

/*************
 * MidiCodec *
 *************/

MidiCodec::MidiCodec( int bufsize, QObject* parent ) : QObject(parent)
{
    CHECK_ERROR(snd_midi_event_new(bufsize, &m_Info));
}

MidiCodec::~MidiCodec()
{
    snd_midi_event_free(m_Info);
}

void
MidiCodec::init()
{
    snd_midi_event_init(m_Info);
}

long
MidiCodec::decode(unsigned char *buf,
                  long count,
                  const snd_seq_event_t *ev)
{
    return CHECK_WARNING(snd_midi_event_decode(m_Info, buf, count, ev));
}

long
MidiCodec::encode(const unsigned char *buf,
                  long count,
                  snd_seq_event_t *ev)
{
    return CHECK_WARNING(snd_midi_event_encode(m_Info, buf, count, ev));
}

long
MidiCodec::encode(int c,
                  snd_seq_event_t *ev)
{
    return CHECK_WARNING(snd_midi_event_encode_byte(m_Info, c, ev));
}

void
MidiCodec::enableRunningStatus(bool enable)
{
    snd_midi_event_no_status(m_Info, enable ? 0 : 1);
}

void
MidiCodec::resetDecoder()
{
    snd_midi_event_reset_decode(m_Info);
}

void
MidiCodec::resetEncoder()
{
    snd_midi_event_reset_encode(m_Info);
}

void
MidiCodec::resizeBuffer(int bufsize)
{
    CHECK_WARNING(snd_midi_event_resize_buffer(m_Info, bufsize));
}

} /* namespace aseqmm */
