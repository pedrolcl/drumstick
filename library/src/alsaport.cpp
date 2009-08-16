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

#include "alsaport.h"
#include "alsaqueue.h"
#include "subscription.h"
#include "alsaclient.h"

namespace aseqmm {

/************/
/* PortInfo */
/************/

PortInfo::PortInfo()
{
    snd_seq_port_info_malloc(&m_Info);
}

PortInfo::PortInfo(const PortInfo& other)
{
    snd_seq_port_info_malloc(&m_Info);
    snd_seq_port_info_copy(m_Info, other.m_Info);
    m_ReadSubscribers = other.m_ReadSubscribers;
    m_WriteSubscribers = other.m_WriteSubscribers;
    m_ClientName = other.m_ClientName;
}

PortInfo::PortInfo(snd_seq_port_info_t* other)
{
    snd_seq_port_info_malloc(&m_Info);
    snd_seq_port_info_copy(m_Info, other);
}

PortInfo::PortInfo(MidiClient* seq, const int client, const int port)
{
    snd_seq_port_info_malloc(&m_Info);
    CHECK_WARNING(snd_seq_get_any_port_info(seq->getHandle(), client, port, m_Info));
}

PortInfo::PortInfo(MidiClient* seq, const int port)
{
    snd_seq_port_info_malloc(&m_Info);
    CHECK_WARNING(snd_seq_get_port_info(seq->getHandle(), port, m_Info));
}

PortInfo::~PortInfo()
{
    snd_seq_port_info_free(m_Info);
    freeSubscribers();
}

PortInfo* PortInfo::clone()
{
    return new PortInfo(m_Info);
}

PortInfo& PortInfo::operator=(const PortInfo& other)
{
    snd_seq_port_info_copy(m_Info, other.m_Info);
    m_ReadSubscribers = other.m_ReadSubscribers;
    m_WriteSubscribers = other.m_WriteSubscribers;
    m_ClientName = other.m_ClientName;
    return *this;
}

int
PortInfo::getClient()
{
    return snd_seq_port_info_get_client(m_Info);
}

int
PortInfo::getPort()
{
    return snd_seq_port_info_get_port(m_Info);
}

const snd_seq_addr_t*
PortInfo::getAddr()
{
    return snd_seq_port_info_get_addr(m_Info);
}

QString
PortInfo::getName()
{
    return QString(snd_seq_port_info_get_name(m_Info));
}

unsigned int
PortInfo::getCapability()
{
    return snd_seq_port_info_get_capability(m_Info);
}

unsigned int
PortInfo::getType()
{
    return snd_seq_port_info_get_type(m_Info);
}

int
PortInfo::getMidiChannels()
{
    return snd_seq_port_info_get_midi_channels(m_Info);
}

int
PortInfo::getMidiVoices()
{
    return snd_seq_port_info_get_midi_voices(m_Info);
}

int
PortInfo::getSynthVoices()
{
    return snd_seq_port_info_get_synth_voices(m_Info);
}

int
PortInfo::getReadUse()
{
    return snd_seq_port_info_get_read_use(m_Info);
}

int
PortInfo::getWriteUse()
{
    return snd_seq_port_info_get_write_use(m_Info);
}

int
PortInfo::getPortSpecified()
{
    return snd_seq_port_info_get_port_specified(m_Info);
}

void
PortInfo::setClient(int client)
{
    snd_seq_port_info_set_client(m_Info, client);
}

void
PortInfo::setPort(int port)
{
    snd_seq_port_info_set_port(m_Info, port);
}

void
PortInfo::setAddr(const snd_seq_addr_t* addr)
{
    snd_seq_port_info_set_addr(m_Info, addr);
}

void
PortInfo::setName(QString const& name)
{
    snd_seq_port_info_set_name(m_Info, name.toLocal8Bit().data());
}

void
PortInfo::setCapability(unsigned int capability)
{
    snd_seq_port_info_set_capability(m_Info, capability);
}

void
PortInfo::setType(unsigned int _type)
{
    snd_seq_port_info_set_type(m_Info, _type);
}

void
PortInfo::setMidiChannels(int channels)
{
    snd_seq_port_info_set_midi_channels(m_Info, channels);
}

void
PortInfo::setMidiVoices(int voices)
{
    snd_seq_port_info_set_midi_voices(m_Info, voices);
}

void
PortInfo::setSynthVoices(int voices)
{
    snd_seq_port_info_set_synth_voices(m_Info, voices);
}

void
PortInfo::setPortSpecified(int val)
{
    snd_seq_port_info_set_port_specified(m_Info, val);
}

SubscribersList
PortInfo::getReadSubscribers() const
{
    return m_ReadSubscribers; // copy
}

SubscribersList
PortInfo::getWriteSubscribers() const
{
    return m_WriteSubscribers; // copy
}

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

void
PortInfo::freeSubscribers()
{
    m_ReadSubscribers.clear();
    m_WriteSubscribers.clear();
}

int
PortInfo::getSizeOfInfo() const
{
    return  snd_seq_port_info_sizeof();
}

bool
PortInfo::getTimestamping()
{
    return (snd_seq_port_info_get_timestamping(m_Info) == 1);
}

bool
PortInfo::getTimestampReal()
{
    return (snd_seq_port_info_get_timestamp_real(m_Info) == 1);
}

int
PortInfo::getTimestampQueue()
{
    return snd_seq_port_info_get_timestamp_queue(m_Info);
}

void
PortInfo::setTimestamping(bool value)
{
    snd_seq_port_info_set_timestamping(m_Info, value?1:0);
}

void
PortInfo::setTimestampReal(bool value)
{
    snd_seq_port_info_set_timestamp_real(m_Info, value?1:0);
}

void
PortInfo::setTimestampQueue(int queueId)
{
    snd_seq_port_info_set_timestamp_queue(m_Info, queueId);
}

/************/
/* MidiPort */
/************/

MidiPort::MidiPort( QObject* parent ) :
    QObject( parent ),
    m_MidiClient( NULL ),
    m_Attached( false )
{}

MidiPort::~MidiPort()
{
    unsubscribeAll();
    detach();
    freeSubscriptions();
}

PortInfo*
MidiPort::getPortInfo()
{
    return &m_Info;
}

SubscriptionsList
MidiPort::getSubscriptions() const
{
    return m_Subscriptions;
}

void
MidiPort::freeSubscriptions()
{
    m_Subscriptions.clear();
}

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

void
MidiPort::subscribe(Subscription* subs)
{
    subs->subscribe(m_MidiClient);
    m_Subscriptions.append(*subs);
    emit subscribed(this, subs);
}

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

void
MidiPort::subscribeTo( PortInfo* info )
{
    Subscription subs;
    subs.setSender(m_Info.getAddr());
    subs.setDest(info->getAddr());
    subscribe(&subs);
}

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

void
MidiPort::subscribeTo( QString const& name )
{
    Subscription subs;
    snd_seq_addr addr;
    if ((m_MidiClient != NULL) && (m_MidiClient->getHandle() != NULL))
    {
        subs.setSender(m_Info.getAddr());
        snd_seq_parse_address(m_MidiClient->getHandle(), &addr, name.toLocal8Bit().data());
        subs.setDest(&addr);
        subscribe(&subs);
    }
}

void
MidiPort::unsubscribeTo( QString const& name )
{
    Subscription subs;
    snd_seq_addr addr;
    if ((m_MidiClient != NULL) && (m_MidiClient->getHandle() != NULL))
    {
        subs.setSender(m_Info.getAddr());
        snd_seq_parse_address(m_MidiClient->getHandle(), &addr, name.toLocal8Bit().data());
        subs.setDest(&addr);
        unsubscribe(&subs);
    }
}

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

void
MidiPort::subscribeFrom( PortInfo* port )
{
    Subscription subs;
    subs.setSender( port->getAddr() );
    subs.setDest( m_Info.getAddr() );
    subscribe(&subs);
}

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

void
MidiPort::subscribeFrom( QString const& name)
{
    Subscription subs;
    snd_seq_addr addr;
    if ((m_MidiClient != NULL) && (m_MidiClient->getHandle() != NULL))
    {
        snd_seq_parse_address(m_MidiClient->getHandle(), &addr, name.toLocal8Bit().data());
        subs.setSender(&addr);
        subs.setDest(m_Info.getAddr());
        subscribe(&subs);
    }
}

void
MidiPort::unsubscribeFrom( QString const& name)
{
    Subscription subs;
    snd_seq_addr addr;
    if ((m_MidiClient != NULL) && (m_MidiClient->getHandle() != NULL))
    {
        snd_seq_parse_address(m_MidiClient->getHandle(), &addr, name.toLocal8Bit().data());
        subs.setSender(&addr);
        subs.setDest(m_Info.getAddr());
        unsubscribe(&subs);
    }
}

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

void
MidiPort::subscribeFromAnnounce()
{
    subscribeFrom(SND_SEQ_CLIENT_SYSTEM, SND_SEQ_PORT_SYSTEM_ANNOUNCE);
}

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

void
MidiPort::applyPortInfo()
{
    if (m_Attached && (m_MidiClient != NULL) && (m_MidiClient->isOpened()))
    {
        CHECK_WARNING(snd_seq_set_port_info( m_MidiClient->getHandle(),
        		                             m_Info.getPort(), m_Info.m_Info ));
    }
}

QString
MidiPort::getPortName()
{
    return m_Info.getName();
}

void
MidiPort::setPortName( QString const& newName )
{
    m_Info.setName(newName);
    applyPortInfo();
}

int
MidiPort::getPortId()
{
    return m_Info.getPort();
}

unsigned int
MidiPort::getCapability()
{
    return m_Info.getCapability();
}

void
MidiPort::setCapability(unsigned int newValue)
{
    m_Info.setCapability(newValue);
    applyPortInfo();
}

unsigned int
MidiPort::getPortType()
{
    return m_Info.getType();
}

void
MidiPort::setPortType( unsigned int newValue)
{
    m_Info.setType( newValue );
    applyPortInfo();
}

int
MidiPort::getMidiChannels()
{
    return m_Info.getMidiChannels();
}

void
MidiPort::setMidiChannels(int newValue)
{
    m_Info.setMidiChannels( newValue );
    applyPortInfo();
}

int
MidiPort::getMidiVoices()
{
    return m_Info.getMidiVoices();
}

void
MidiPort::setMidiVoices(int newValue)
{
    m_Info.setMidiVoices( newValue );
    applyPortInfo();
}

int
MidiPort::getSynthVoices()
{
    return m_Info.getSynthVoices();
}

void
MidiPort::setSynthVoices(int newValue)
{
    m_Info.setSynthVoices( newValue );
    applyPortInfo();
}

bool
MidiPort::getTimestamping()
{
	return m_Info.getTimestamping();
}

bool
MidiPort::getTimestampReal()
{
	return m_Info.getTimestampReal();
}

int
MidiPort::getTimestampQueue()
{
	return m_Info.getTimestampQueue();
}

void
MidiPort::setTimestamping(bool value)
{
	m_Info.setTimestamping(value);
	applyPortInfo();
}

void
MidiPort::setTimestampReal(bool value)
{
	m_Info.setTimestampReal(value);
	applyPortInfo();
}

void
MidiPort::setTimestampQueue(int queueId)
{
	m_Info.setTimestampQueue(queueId);
	applyPortInfo();
}

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

void
MidiPort::detach()
{
    if (m_Attached && (m_MidiClient != NULL)) {
        m_MidiClient->portDetach(this);
        m_Attached = false;
        emit detached(this);
    }
}

void
MidiPort::updateSubscribers()
{
    m_Info.readSubscribers(m_MidiClient);
}

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

void
MidiPort::updateConnectionsTo(const PortInfoList& ports)
{
    PortInfoList subs(getReadSubscribers());
    PortInfoList::ConstIterator i;
    for (i = subs.begin(); i != subs.end(); ++i) {
        PortInfo s = *i;
        if (!containsAddress(s.getAddr(), ports)) {
            unsubscribeTo(s.getAddr());
        }
    }
    for (i = ports.begin(); i != ports.end(); ++i) {
        PortInfo p = *i;
        if (!containsAddress(p.getAddr(), subs)) {
            subscribeTo(&p);
        }
    }
}

void
MidiPort::updateConnectionsFrom(const PortInfoList& ports)
{
    PortInfoList subs(getWriteSubscribers());
    PortInfoList::ConstIterator i;
    for (i = subs.begin(); i != subs.end(); ++i) {
        PortInfo s = *i;
        if (!containsAddress(s.getAddr(), ports)) {
            unsubscribeFrom(s.getAddr());
        }
    }
    for (i = ports.begin(); i != ports.end(); ++i) {
        PortInfo p = *i;
        if (!containsAddress(p.getAddr(), subs)) {
            subscribeFrom(&p);
        }
    }
}

} /* namespace aseqmm; */
