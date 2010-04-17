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

#include "alsaqueue.h"
#include "alsaclient.h"

/**
 * @file alsaport.cpp
 * Implementation of classes managing ALSA Sequencer ports.
 */

namespace drumstick {

/**
 * @addtogroup ALSAPort
 * @{
 *
 * Ports are the endpoints of the MIDI connections.
 *
 * Ports can be readable, writable, or both. They can be private for the owner
 * application, or can be subscribed by a third application.
 *
 * The ALSA sequencer readable ports are equivalent to the MIDI OUT ports in the
 * real world. Similarly, the writable ports are equivalent to the MIDI IN ones.
 *
 * Classes:
 *
 * PortInfo is a container to retrieve and change some properties about the ALSA
 * MIDI ports.
 *
 * MidiPort represents an ALSA MIDI port.
 *
 * @see http://www.alsa-project.org/alsa-doc/alsa-lib/group___seq_port.html
 * @}
 */

/**
 * Default constructor.
 */
PortInfo::PortInfo()
{
    snd_seq_port_info_malloc(&m_Info);
}

/**
 * Copy constructor
 * @param other Another PortInfo object reference
 */
PortInfo::PortInfo(const PortInfo& other)
{
    snd_seq_port_info_malloc(&m_Info);
    snd_seq_port_info_copy(m_Info, other.m_Info);
    m_ReadSubscribers = other.m_ReadSubscribers;
    m_WriteSubscribers = other.m_WriteSubscribers;
    m_ClientName = other.m_ClientName;
}

/**
 * Constructor
 * @param other An ALSA port info object pointer
 */
PortInfo::PortInfo(snd_seq_port_info_t* other)
{
    snd_seq_port_info_malloc(&m_Info);
    snd_seq_port_info_copy(m_Info, other);
}

/**
 * Constructor
 * @param seq A MidiClient instance
 * @param client An existing client number
 * @param port An existing port number
 */
PortInfo::PortInfo(MidiClient* seq, const int client, const int port)
{
    snd_seq_port_info_malloc(&m_Info);
    CHECK_WARNING(snd_seq_get_any_port_info(seq->getHandle(), client, port, m_Info));
}

/**
 * Constructor
 * @param seq A MidiClient instance
 * @param port An existing port number
 */
PortInfo::PortInfo(MidiClient* seq, const int port)
{
    snd_seq_port_info_malloc(&m_Info);
    CHECK_WARNING(snd_seq_get_port_info(seq->getHandle(), port, m_Info));
}

/**
 * Destructor
 */
PortInfo::~PortInfo()
{
    snd_seq_port_info_free(m_Info);
    freeSubscribers();
}

/**
 * Copy the current object
 * @return A pointer to the new object
 */
PortInfo* PortInfo::clone()
{
    return new PortInfo(m_Info);
}

/**
 * Assignment operator
 * @param other Another PortInfo object reference
 * @return This object
 */
PortInfo& PortInfo::operator=(const PortInfo& other)
{
    snd_seq_port_info_copy(m_Info, other.m_Info);
    m_ReadSubscribers = other.m_ReadSubscribers;
    m_WriteSubscribers = other.m_WriteSubscribers;
    m_ClientName = other.m_ClientName;
    return *this;
}

/**
 * Gets the client number
 * @return The client number
 * @see setClient()
 */
int
PortInfo::getClient()
{
    return snd_seq_port_info_get_client(m_Info);
}

/**
 * Gets the port number
 * @return The port number
 * @see setPort()
 */
int
PortInfo::getPort()
{
    return snd_seq_port_info_get_port(m_Info);
}

/**
 * Gets the address record for this port
 * @return A pointer to the address record
 * @see setAddr()
 */
const snd_seq_addr_t*
PortInfo::getAddr()
{
    return snd_seq_port_info_get_addr(m_Info);
}

/**
 * Gets the port name
 * @return The port name
 * @see setName()
 */
QString
PortInfo::getName()
{
    return QString(snd_seq_port_info_get_name(m_Info));
}

/**
 * Gets the capabilities bitmap
 * @return The capabilities bitmap
 * @see setCapability()
 */
unsigned int
PortInfo::getCapability()
{
    return snd_seq_port_info_get_capability(m_Info);
}

/**
 * Gets the port type
 * @return The port type
 * @see setType()
 */
unsigned int
PortInfo::getType()
{
    return snd_seq_port_info_get_type(m_Info);
}

/**
 * Gets the MIDI channels
 * @return The MIDI channels
 * @see setMidiChannels()
 */
int
PortInfo::getMidiChannels()
{
    return snd_seq_port_info_get_midi_channels(m_Info);
}

/**
 * Gets the MIDI voices
 * @return The MIDI voices
 * @see setMidiVoices()
 */
int
PortInfo::getMidiVoices()
{
    return snd_seq_port_info_get_midi_voices(m_Info);
}

/**
 * Gets the synth voices
 * @return The synth voices
 * @see setSynthVoices()
 */
int
PortInfo::getSynthVoices()
{
    return snd_seq_port_info_get_synth_voices(m_Info);
}

/**
 * Get the number of read subscriptions.
 * @return The number of read subscriptions.
 */
int
PortInfo::getReadUse()
{
    return snd_seq_port_info_get_read_use(m_Info);
}

/**
 * Gets the number of write subscriptions.
 * @return The number of write subscriptions.
 */
int
PortInfo::getWriteUse()
{
    return snd_seq_port_info_get_write_use(m_Info);
}

/**
 * Gets the port-specified mode.
 * @return The port-specified mode.
 * @see setPortSpecified()
 */
int
PortInfo::getPortSpecified()
{
    return snd_seq_port_info_get_port_specified(m_Info);
}

/**
 * Sets the client number
 * @param client The client number
 * @see getClient()
 */
void
PortInfo::setClient(int client)
{
    snd_seq_port_info_set_client(m_Info, client);
}

/**
 * Set the port number
 * @param port The port number
 * @see getPort()
 */
void
PortInfo::setPort(int port)
{
    snd_seq_port_info_set_port(m_Info, port);
}

/**
 * Sets the address record
 * @param addr An address record pointer
 * @see getAddr()
 */
void
PortInfo::setAddr(const snd_seq_addr_t* addr)
{
    snd_seq_port_info_set_addr(m_Info, addr);
}

/**
 * Sets the port name
 * @param name The port name
 * @see getName()
 */
void
PortInfo::setName(QString const& name)
{
    snd_seq_port_info_set_name(m_Info, name.toLocal8Bit().data());
}

/**
 * Sets the capability bitmap.
 *
 * Each port has the capability bitmaps to specify the access of
 * the port from other clients. The capability bit flags are:
 * <ul>
 * <li>SND_SEQ_PORT_CAP_READ Readable from this port</li>
 * <li>SND_SEQ_PORT_CAP_WRITE Writable to this port</li>
 * <li>SND_SEQ_PORT_CAP_DUPLEX Read/write duplex access is supported</li>
 * <li>SND_SEQ_PORT_CAP_SUBS_READ Read subscription is allowed</li>
 * <li>SND_SEQ_PORT_CAP_SUBS_WRITE Write subscription is allowed</li>
 * <li>SND_SEQ_PORT_CAP_NO_EXPORT Subscription management from 3rd clients is disallowed</li>
 * </ul>
 * @param capability The capability bitmap.
 * @see getCapability()
 */
void
PortInfo::setCapability(unsigned int capability)
{
    snd_seq_port_info_set_capability(m_Info, capability);
}

/**
 * Sets the port type.
 *
 * The port type is defined combining some of the type bit flags:
 * <ul>
 * <li>SND_SEQ_PORT_TYPE_SPECIFIC Hardware specific port</li>
 * <li>SND_SEQ_PORT_TYPE_MIDI_GENERIC Generic MIDI device</li>
 * <li>SND_SEQ_PORT_TYPE_MIDI_GM General MIDI compatible device</li>
 * <li>SND_SEQ_PORT_TYPE_MIDI_GM2 General MIDI 2 compatible device</li>
 * <li>SND_SEQ_PORT_TYPE_MIDI_GS GS compatible device</li>
 * <li>SND_SEQ_PORT_TYPE_MIDI_XG XG compatible device</li>
 * <li>SND_SEQ_PORT_TYPE_MIDI_MT32 MT-32 compatible device</li>
 * <li>SND_SEQ_PORT_TYPE_HARDWARE Implemented in hardware</li>
 * <li>SND_SEQ_PORT_TYPE_SOFTWARE Implemented in software</li>
 * <li>SND_SEQ_PORT_TYPE_SYNTHESIZER Generates sound</li>
 * <li>SND_SEQ_PORT_TYPE_PORT Connects to other device(s)</li>
 * <li>SND_SEQ_PORT_TYPE_APPLICATION Application (sequencer/editor)</li>
 * </ul>
 * @param type The port type bitmap
 * @see getType()
 */
void
PortInfo::setType(unsigned int type)
{
    snd_seq_port_info_set_type(m_Info, type);
}

/**
 * Set the MIDI channels
 * @param channels The MIDI channels
 * @see getMidiChannels()
 */
void
PortInfo::setMidiChannels(int channels)
{
    snd_seq_port_info_set_midi_channels(m_Info, channels);
}

/**
 * Sets the MIDI voices
 * @param voices The MIDI voices
 * @see getMidiVoices()
 */
void
PortInfo::setMidiVoices(int voices)
{
    snd_seq_port_info_set_midi_voices(m_Info, voices);
}

/**
 * Sets the synth voices
 * @param voices The synth voices
 * @see getSynthVoices()
 */
void
PortInfo::setSynthVoices(int voices)
{
    snd_seq_port_info_set_synth_voices(m_Info, voices);
}

/**
 * Sets the port-specified mode
 * @param val The port-specified mode.
 * @see getPortSpecified()
 */
void
PortInfo::setPortSpecified(int val)
{
    snd_seq_port_info_set_port_specified(m_Info, val);
}

/**
 * Gets the list of read subscribers
 * @return The list of read subscribers
 */
SubscribersList
PortInfo::getReadSubscribers() const
{
    return m_ReadSubscribers; // copy
}

/**
 * Gets the list of write subscribers
 * @return The list of write subscribers
 */
SubscribersList
PortInfo::getWriteSubscribers() const
{
    return m_WriteSubscribers; // copy
}

/**
 * Obtains the port subscribers lists
 * @param seq An opened MidiClient instance
 */
void
PortInfo::readSubscribers(MidiClient* seq)
{
    Subscriber subs;
    snd_seq_addr_t tmp;
    freeSubscribers();
    tmp.client = getClient();
    tmp.port = getPort();
    // Read subs
    subs.setType(SND_SEQ_QUERY_SUBS_READ);
    subs.setIndex(0);
    subs.setRoot(&tmp);
    while (snd_seq_query_port_subscribers(seq->getHandle(), subs.m_Info) >= 0)
    {
        m_ReadSubscribers.append(subs);
        subs.setIndex(subs.getIndex() + 1);
    }
    // Write subs
    subs.setType(SND_SEQ_QUERY_SUBS_WRITE);
    subs.setIndex(0);
    subs.setRoot(&tmp);
    while (snd_seq_query_port_subscribers(seq->getHandle(), subs.m_Info) >= 0)
    {
        m_WriteSubscribers.append(subs);
        subs.setIndex(subs.getIndex() + 1);
    }
}

/**
 * Releases the subscribers lists
 */
void
PortInfo::freeSubscribers()
{
    m_ReadSubscribers.clear();
    m_WriteSubscribers.clear();
}

/**
 * Gets the size of the ALSA info object
 * @return The size of the object
 */
int
PortInfo::getSizeOfInfo() const
{
    return  snd_seq_port_info_sizeof();
}

/**
 * Gets the timestamping mode
 * @return The timestamping mode
 * @see setTimestamping()
 */
bool
PortInfo::getTimestamping()
{
    return (snd_seq_port_info_get_timestamping(m_Info) == 1);
}

/**
 * Gets the timestamping real mode
 * @return The timestamping real mode
 * @see setTimestampReal()
 */
bool
PortInfo::getTimestampReal()
{
    return (snd_seq_port_info_get_timestamp_real(m_Info) == 1);
}

/**
 * Gets the timestamping queue number
 * @return The timestamping queue number
 * @see setTimestampQueue()
 */
int
PortInfo::getTimestampQueue()
{
    return snd_seq_port_info_get_timestamp_queue(m_Info);
}

/**
 * Sets the timestamping mode
 * @param value The timestamping mode
 * @see getTimestamping()
 */
void
PortInfo::setTimestamping(bool value)
{
    snd_seq_port_info_set_timestamping(m_Info, value?1:0);
}

/**
 * Sets the timestamping real mode
 * @param value The timestamping real mode
 * @see getTimestampReal()
 */
void
PortInfo::setTimestampReal(bool value)
{
    snd_seq_port_info_set_timestamp_real(m_Info, value?1:0);
}

/**
 * Sets the timestamp queue number
 * @param queueId The timestamp queue number
 * @see getTimestampQueue()
 */
void
PortInfo::setTimestampQueue(int queueId)
{
    snd_seq_port_info_set_timestamp_queue(m_Info, queueId);
}


/**
 * Constructor
 * @param parent An optional parent object
 */
MidiPort::MidiPort( QObject* parent ) :
    QObject( parent ),
    m_MidiClient( NULL ),
    m_Attached( false )
{}

/**
 * Destructor.
 *
 * All subscriptions are released.
 */
MidiPort::~MidiPort()
{
    unsubscribeAll();
    detach();
    freeSubscriptions();
}

/**
 * Gets the PortInfo object pointer
 */
PortInfo*
MidiPort::getPortInfo()
{
    return &m_Info;
}

/**
 * Gets the list of susbcriptions
 * @return  The list of susbcriptions
 */
SubscriptionsList
MidiPort::getSubscriptions() const
{
    return m_Subscriptions;
}

/**
 * Releases the lists of subscriptions.
 */
void
MidiPort::freeSubscriptions()
{
    m_Subscriptions.clear();
}

/**
 * Sets the MidiClient
 * @param seq A MidiClient object pointer
 */
void
MidiPort::setMidiClient( MidiClient* seq )
{
    if (m_MidiClient != seq)
    {
        m_MidiClient = seq;
        emit midiClientChanged( this, m_MidiClient );
        applyPortInfo();
    }
}

/**
 * Subscribe a Subscription object
 * @param subs A Subscription object pointer
 */
void
MidiPort::subscribe(Subscription* subs)
{
    subs->subscribe(m_MidiClient);
    m_Subscriptions.append(*subs);
    emit subscribed(this, subs);
}

/**
 * Unsubscribe a Subscription object
 * @param subs A Subscription object pointer
 */
void
MidiPort::unsubscribe( Subscription* subs )
{
    Subscription subs2;
    if (m_MidiClient == NULL)
    {
        return;
    }
    subs->unsubscribe(m_MidiClient);
    SubscriptionsList::iterator it;
    for(it = m_Subscriptions.begin(); it != m_Subscriptions.end(); ++it)
    {
        subs2 = (*it);
        if ((subs2.getSender()->client == subs->getSender()->client) &&
                (subs2.getSender()->port == subs->getSender()->port) &&
                (subs2.getDest()->client == subs->getDest()->client) &&
                (subs2.getDest()->port == subs->getDest()->port))
        {
            m_Subscriptions.erase(it);
            break;
        }
    }
}

/**
 * Subscribe to another port destination
 * @param info A PortInfo object pointer
 */
void
MidiPort::subscribeTo( PortInfo* info )
{
    Subscription subs;
    subs.setSender(m_Info.getAddr());
    subs.setDest(info->getAddr());
    subscribe(&subs);
}

/**
 * Susbcribe to another port destination
 * @param client ALSA client number
 * @param port ALSA port number
 */
void
MidiPort::subscribeTo( int client, int port )
{
    Subscription subs;
    snd_seq_addr addr;
    addr.client = client;
    addr.port = port;
    subs.setSender(m_Info.getAddr());
    subs.setDest(&addr);
    subscribe(&subs);
}

/**
 * Subscribe to another port destination
 * @param name A string representing a client:port pair
 */
void
MidiPort::subscribeTo( QString const& name )
{
    Subscription subs;
    snd_seq_addr addr;
    if ((m_MidiClient != NULL) && (m_MidiClient->getHandle() != NULL))
    {
        subs.setSender(m_Info.getAddr());
        if (m_MidiClient->parseAddress(name, addr)) {
            subs.setDest(&addr);
            subscribe(&subs);
        }
    }
}

/**
 * Unsubscribe a destination port
 * @param name A string representing a client:port pair
 */
void
MidiPort::unsubscribeTo( QString const& name )
{
    Subscription subs;
    snd_seq_addr addr;
    if ((m_MidiClient != NULL) && (m_MidiClient->getHandle() != NULL))
    {
        subs.setSender(m_Info.getAddr());
        if (m_MidiClient->parseAddress(name, addr)) {
            subs.setDest(&addr);
            unsubscribe(&subs);
        }
    }
}

/**
 * Unsubscribe a destination port
 * @param port A PortInfo object pointer
 */
void
MidiPort::unsubscribeTo( PortInfo* port )
{
    Subscription subs;
    if ((m_MidiClient != NULL) && (m_MidiClient->getHandle() != NULL))
    {
        subs.setSender(m_Info.getAddr());
        subs.setDest(port->getAddr());
        unsubscribe(&subs);
    }
}

/**
 * Unsubscribe a destination port
 * @param addr An ALSA address record pointer
 */
void
MidiPort::unsubscribeTo( const snd_seq_addr_t* addr )
{
    Subscription subs;
    if ((m_MidiClient != NULL) && (m_MidiClient->getHandle() != NULL))
    {
        subs.setSender(m_Info.getAddr());
        subs.setDest(addr);
        unsubscribe(&subs);
    }
}

/**
 * Subscribe a source port
 * @param port A PortInfo object pointer
 */
void
MidiPort::subscribeFrom( PortInfo* port )
{
    Subscription subs;
    subs.setSender( port->getAddr() );
    subs.setDest( m_Info.getAddr() );
    subscribe(&subs);
}

/**
 * Subscribe a source port
 * @param client ALSA client number
 * @param port ALSA port number
 */
void
MidiPort::subscribeFrom( int client, int port )
{
    Subscription subs;
    snd_seq_addr addr;
    addr.client = client;
    addr.port = port;
    subs.setSender(&addr);
    subs.setDest(m_Info.getAddr());
    subscribe(&subs);
}

/**
 * Subscribe a source port
 * @param name A string representing a client:port pair
 */
void
MidiPort::subscribeFrom( QString const& name )
{
    Subscription subs;
    snd_seq_addr addr;
    if ((m_MidiClient != NULL) && (m_MidiClient->getHandle() != NULL))
    {
        if (m_MidiClient->parseAddress(name, addr)) {
            subs.setSender(&addr);
            subs.setDest(m_Info.getAddr());
            subscribe(&subs);
        }
    }
}

/**
 * Unsubscribe a source port
 * @param name A string representing a client:port pair
 */
void
MidiPort::unsubscribeFrom( QString const& name )
{
    Subscription subs;
    snd_seq_addr addr;
    if ((m_MidiClient != NULL) && (m_MidiClient->getHandle() != NULL))
    {
        if (m_MidiClient->parseAddress(name, addr)) {
            subs.setSender(&addr);
            subs.setDest(m_Info.getAddr());
            unsubscribe(&subs);
        }
    }
}

/**
 * Unsubscribe a source port
 * @param port A PortInfo object pointer
 */
void
MidiPort::unsubscribeFrom( PortInfo* port )
{
    Subscription subs;
    if ((m_MidiClient != NULL) && (m_MidiClient->getHandle() != NULL))
    {
        subs.setSender(port->getAddr());
        subs.setDest(m_Info.getAddr());
        unsubscribe(&subs);
    }
}

/**
 * Unsubscribe a source port
 * @param addr An ALSA address record pointer
 */
void
MidiPort::unsubscribeFrom( const snd_seq_addr_t* addr )
{
    Subscription subs;
    if ((m_MidiClient != NULL) && (m_MidiClient->getHandle() != NULL))
    {
        subs.setSender(addr);
        subs.setDest(m_Info.getAddr());
        unsubscribe(&subs);
    }
}

/**
 * Subscribe from the System:announce port
 */
void
MidiPort::subscribeFromAnnounce()
{
    subscribeFrom(SND_SEQ_CLIENT_SYSTEM, SND_SEQ_PORT_SYSTEM_ANNOUNCE);
}

/**
 * Unsubscribe all subscriptions
 */
void
MidiPort::unsubscribeAll()
{
    if (m_MidiClient == NULL) {
        return;
    }
    SubscriptionsList::Iterator it;
    for( it = m_Subscriptions.begin(); it != m_Subscriptions.end(); ++it) {
        Subscription s = (*it);
        s.unsubscribe(m_MidiClient);
    }
    m_Subscriptions.clear();
}

/**
 * Applies all the the delayed PortInfo changes to the MIDI port object
 */
void
MidiPort::applyPortInfo()
{
    if (m_Attached && (m_MidiClient != NULL) && (m_MidiClient->isOpened()))
    {
        CHECK_WARNING(snd_seq_set_port_info( m_MidiClient->getHandle(),
        		                             m_Info.getPort(), m_Info.m_Info ));
    }
}

/**
 * Gets the port name
 * @return The port name
 */
QString
MidiPort::getPortName()
{
    return m_Info.getName();
}

/**
 * Sets the port name
 * @param newName The new port name
 */
void
MidiPort::setPortName( QString const& newName )
{
    m_Info.setName(newName);
    applyPortInfo();
}

/**
 * Gets the port number
 * @return The port number
 */
int
MidiPort::getPortId()
{
    return m_Info.getPort();
}

/**
 * Gets the port capabilities
 * @return The capabilities bitmap
 * @see PortInfo::getCapability()
 */
unsigned int
MidiPort::getCapability()
{
    return m_Info.getCapability();
}

/**
 * Sets the port capabilities
 * @param newValue The capabilities bitmap
 * @see PortInfo::setCapability()
 */
void
MidiPort::setCapability(unsigned int newValue)
{
    m_Info.setCapability(newValue);
    applyPortInfo();
}

/**
 * Gets the port type
 * @return The port type bitmap
 * @see PortInfo::getType()
 */
unsigned int
MidiPort::getPortType()
{
    return m_Info.getType();
}

/**
 * Sets the port type bitmap
 * @param newValue The port type flags bitmap
 * @see PortInfo::setType()
 */
void
MidiPort::setPortType( unsigned int newValue)
{
    m_Info.setType( newValue );
    applyPortInfo();
}

/**
 * Gets the MIDI channels
 * @return The MIDI channels
 */
int
MidiPort::getMidiChannels()
{
    return m_Info.getMidiChannels();
}

/**
 * Sets the MIDI channels
 * @param newValue The MIDI channels
 */
void
MidiPort::setMidiChannels(int newValue)
{
    m_Info.setMidiChannels( newValue );
    applyPortInfo();
}

/**
 * Gets the MIDI voices
 * @return The MIDI voices
 */
int
MidiPort::getMidiVoices()
{
    return m_Info.getMidiVoices();
}

/**
 * Sets the MIDI voices
 * @param newValue The MIDI voices
 */
void
MidiPort::setMidiVoices(int newValue)
{
    m_Info.setMidiVoices( newValue );
    applyPortInfo();
}

/**
 * Gets the synth voices
 * @return The synth voices
 */
int
MidiPort::getSynthVoices()
{
    return m_Info.getSynthVoices();
}

/**
 * Sets the synth voices
 * @param newValue The synth voices
 */
void
MidiPort::setSynthVoices(int newValue)
{
    m_Info.setSynthVoices( newValue );
    applyPortInfo();
}

/**
 * Gets the timestamping mode
 * @return The timestamping mode
 */
bool
MidiPort::getTimestamping()
{
	return m_Info.getTimestamping();
}

/**
 * Gets the timestamp real mode
 * @return The timestamp real mode
 */
bool
MidiPort::getTimestampReal()
{
	return m_Info.getTimestampReal();
}

/**
 * Gets the timestamp queue number
 * @return The timestamp queue number
 */
int
MidiPort::getTimestampQueue()
{
	return m_Info.getTimestampQueue();
}

/**
 * Sets the timestamping mode
 * @param value The timestamping mode
 */
void
MidiPort::setTimestamping(bool value)
{
	m_Info.setTimestamping(value);
	applyPortInfo();
}

/**
 * Sets the timestamp real mode
 * @param value The timestamp real mode
 */
void
MidiPort::setTimestampReal(bool value)
{
	m_Info.setTimestampReal(value);
	applyPortInfo();
}

/**
 * Sets the timestamp queue number
 * @param queueId The queue number
 */
void
MidiPort::setTimestampQueue(int queueId)
{
	m_Info.setTimestampQueue(queueId);
	applyPortInfo();
}

/**
 * Attach the port to a MidiClient instance
 * @param seq A MidiClient object pointer
 */
void
MidiPort::attach( MidiClient* seq )
{
    if (!m_Attached && (seq != NULL)) {
        m_MidiClient = seq;
        m_MidiClient->portAttach(this);
        m_Attached = true;
        emit attached(this);
    }
}

/**
 * Detach the port from any MidiClient instance previously attached
 */
void
MidiPort::detach()
{
    if (m_Attached && (m_MidiClient != NULL)) {
        m_MidiClient->portDetach(this);
        m_Attached = false;
        emit detached(this);
    }
}

/**
 * Update the subscribers list in the PortInfo member
 */
void
MidiPort::updateSubscribers()
{
    m_Info.readSubscribers(m_MidiClient);
}

/**
 * Gets the list of read subscribers
 * @return The list of read subscribers
 */
PortInfoList
MidiPort::getReadSubscribers()
{
    const SubscribersList subs(m_Info.getReadSubscribers());
    PortInfoList lst;
    SubscribersList::ConstIterator it;
    for(it = subs.constBegin(); it != subs.constEnd(); ++it) {
        Subscriber s = *it;
        int client = s.getAddr()->client;
        if ((client != SND_SEQ_CLIENT_SYSTEM) && (client != m_Info.getClient())) {
            int port = s.getAddr()->port;
            PortInfo p(m_MidiClient, client, port);
            if ((p.getCapability() & SND_SEQ_PORT_CAP_NO_EXPORT) == 0) {
            	p.setClientName(m_MidiClient->getClientName(client));
                lst << p;
            }
        }
    }
    return lst;
}

/**
 * Gets the list of write subscribers
 * @return The list of write subscribers
 */
PortInfoList
MidiPort::getWriteSubscribers()
{
    const SubscribersList subs(m_Info.getWriteSubscribers());
    PortInfoList lst;
    SubscribersList::ConstIterator it;
    for(it = subs.constBegin(); it != subs.constEnd(); ++it) {
        Subscriber s = *it;
        int client = s.getAddr()->client;
        if ((client != SND_SEQ_CLIENT_SYSTEM) && (client != m_Info.getClient())) {
            int port = s.getAddr()->port;
            PortInfo p(m_MidiClient, client, port);
            if ((p.getCapability() & SND_SEQ_PORT_CAP_NO_EXPORT) == 0) {
            	p.setClientName(m_MidiClient->getClientName(client));
                lst << p;
            }
        }
    }
    return lst;
}

/**
 * Checks if the provided address is included in the port list
 * @param addr ALSA address record pointer
 * @param lst List of port information containers
 * @return True if the address is found
 */
bool
MidiPort::containsAddress(const snd_seq_addr_t* addr, const PortInfoList& lst)
{
    PortInfoList::ConstIterator i;
    for( i = lst.begin(); i != lst.end(); ++i) {
        PortInfo p = *i;
        if ((p.getAddr()->client == addr->client) &&
            (p.getAddr()->port == addr->port)) {
            return true;
        }
    }
    return false;
}

/**
 * Update the write subscriptions
 * @param ports List of writable ports to be subscribed
 */
void
MidiPort::updateConnectionsTo(const PortInfoList& ports)
{
    PortInfoList subs(getReadSubscribers());
    PortInfoList::ConstIterator i;
    for (i = subs.constBegin(); i != subs.constEnd(); ++i) {
        PortInfo s = *i;
        if (!containsAddress(s.getAddr(), ports)) {
            unsubscribeTo(s.getAddr());
        }
    }
    for (i = ports.constBegin(); i != ports.constEnd(); ++i) {
        PortInfo p = *i;
        if (!containsAddress(p.getAddr(), subs)) {
            subscribeTo(&p);
        }
    }
}

/**
 * Update the read susbcriptions
 * @param ports List of readable ports to be subscribed
 */
void
MidiPort::updateConnectionsFrom(const PortInfoList& ports)
{
    PortInfoList subs(getWriteSubscribers());
    PortInfoList::ConstIterator i;
    for (i = subs.constBegin(); i != subs.constEnd(); ++i) {
        PortInfo s = *i;
        if (!containsAddress(s.getAddr(), ports)) {
            unsubscribeFrom(s.getAddr());
        }
    }
    for (i = ports.constBegin(); i != ports.constEnd(); ++i) {
        PortInfo p = *i;
        if (!containsAddress(p.getAddr(), subs)) {
            subscribeFrom(&p);
        }
    }
}

} /* namespace drumstick; */
