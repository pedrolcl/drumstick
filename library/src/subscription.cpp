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

#include "alsaclient.h"

/**
 * @file subscription.cpp
 * Implementation of classes managing ALSA sequencer subscriptions
 */

namespace drumstick {

/**
 * @addtogroup ALSASubscription
 * @{
 *
 * Subscriptions are virtual MIDI cables between readable and writable ports.
 *
 * The ALSA sequencer readable ports are equivalent to the MIDI OUT ports in the
 * real world. Similarly, the writable ports are equivalent to the MIDI IN ones.
 * Subscriptions, like real MIDI cables, always involve a readable port (source)
 * and a writable port (destination).
 *
 * Classes:
 *
 * Subscriber: This class is used to enumerate the subscribers of a given (root) port.
 *
 * Subscription: This class represents a connection between two ports.
 *
 * @see http://www.alsa-project.org/alsa-doc/alsa-lib/group___seq_subscribe.html
 * @}
 */

/**
 * Default constructor
 */
Subscriber::Subscriber()
{
    snd_seq_query_subscribe_malloc(&m_Info);
}

/**
 * Copy constructor
 * @param other Existing Subscriber object reference
 */
Subscriber::Subscriber(const Subscriber& other)
{
    snd_seq_query_subscribe_malloc(&m_Info);
    snd_seq_query_subscribe_copy(m_Info, other.m_Info);
}

/**
 * Constructor
 * @param other Pointer to an ALSA query subscribe object
 */
Subscriber::Subscriber(snd_seq_query_subscribe_t* other)
{
    snd_seq_query_subscribe_malloc(&m_Info);
    snd_seq_query_subscribe_copy(m_Info, other);
}

/**
 * Destructor
 */
Subscriber::~Subscriber()
{
    snd_seq_query_subscribe_free(m_Info);
}

/**
 * Copy the current object
 * @return Pointer to the new object
 */
Subscriber* Subscriber::clone()
{
    return new Subscriber(m_Info);
}

/**
 * Assignment operator
 * @param other Existing Subscriber object reference
 * @return This object
 */
Subscriber& Subscriber::operator=(const Subscriber& other)
{
    snd_seq_query_subscribe_copy(m_Info, other.m_Info);
    return *this;
}

/**
 * Gets the subscriber's client number
 * @return Client number
 */
int
Subscriber::getClient()
{
    return snd_seq_query_subscribe_get_client(m_Info);
}

/**
 * Gets the subscriober's port number
 * @return Port number
 */
int
Subscriber::getPort()
{
    return snd_seq_query_subscribe_get_port(m_Info);
}

/**
 * Gets the subscriber's root address
 * @return Pointer to the ALSA client/port address
 */
const snd_seq_addr_t*
Subscriber::getRoot()
{
    return snd_seq_query_subscribe_get_root(m_Info);
}

/**
 * Gets the subscription type (read or write).
 * <ul>
 * <li>SND_SEQ_QUERY_SUBS_READ: read subscriptions</li>
 * <li>SND_SEQ_QUERY_SUBS_WRITE: write subscriptions</li>
 * </ul>
 * @return Subscription type
 */
snd_seq_query_subs_type_t
Subscriber::getType()
{
    return snd_seq_query_subscribe_get_type(m_Info);
}

/**
 * Gets the index of the subscriber container
 * @return Index of the subscriber
 */
int
Subscriber::getIndex()
{
    return snd_seq_query_subscribe_get_index(m_Info);
}

/**
 * Gets the number of subscribers returned by a query operation
 * @return Number of subscribers
 */
int
Subscriber::getNumSubs()
{
    return snd_seq_query_subscribe_get_num_subs(m_Info);
}

/**
 * Gets the subscriber's address
 * @return Pointer to the ALSA address record
 */
const snd_seq_addr_t*
Subscriber::getAddr()
{
    return snd_seq_query_subscribe_get_addr(m_Info);
}

/**
 * Gets the subscriber's queue number
 * @return Queue number
 */
int
Subscriber::getQueue()
{
    return snd_seq_query_subscribe_get_queue(m_Info);
}

/**
 * Gets the subscriber's exclusive flag
 * @return Exclusive flag
 */
bool
Subscriber::getExclusive()
{
    return (snd_seq_query_subscribe_get_exclusive(m_Info) != 0);
}

/**
 * Gets the susbcriber's time-update flag
 * @return Time update flag
 */
bool
Subscriber::getTimeUpdate()
{
    return (snd_seq_query_subscribe_get_time_update(m_Info) != 0);
}

/**
 * Gets the subscriber's time real time-stamp flag
 * @return Time real flag
 */
bool
Subscriber::getTimeReal()
{
    return (snd_seq_query_subscribe_get_time_real(m_Info) != 0);
}

/**
 * Sets the subscriber's client number
 * @param client Client number
 */
void
Subscriber::setClient(int client)
{
    snd_seq_query_subscribe_set_client(m_Info, client);
}

/**
 * Sets the subscriber's port number
 * @param port Port number
 */
void
Subscriber::setPort(int port)
{
    snd_seq_query_subscribe_set_port(m_Info, port);
}

/**
 * Sets the subscriber's root address
 * @param addr Pointer to the root ALSA address record
 */
void
Subscriber::setRoot(snd_seq_addr_t* addr)
{
    snd_seq_query_subscribe_set_root(m_Info, addr);
}

/**
 * Sets the subscription type
 * <ul>
 * <li>SND_SEQ_QUERY_SUBS_READ: read subscriptions</li>
 * <li>SND_SEQ_QUERY_SUBS_WRITE: write subscriptions</li>
 * </ul>
 * @param type Subscription type
 */
void
Subscriber::setType(snd_seq_query_subs_type_t type)
{
    snd_seq_query_subscribe_set_type(m_Info, type);
}

/**
 * Sets the index of the subscriber
 * @param index Subscriber index
 */
void
Subscriber::setIndex(int index)
{
    snd_seq_query_subscribe_set_index(m_Info, index);
}

/**
 * Gets the size of the ALSA query subscriber object
 * @return Size of the ALSA object
 */
int 
Subscriber::getSizeOfInfo() const
{
    return snd_seq_query_subscribe_sizeof();
}

/**
 * Default constructor
 */
Subscription::Subscription()
{
    snd_seq_port_subscribe_malloc(&m_Info);
}

/**
 * Copy constructor
 * @param other Existing Subscription object reference
 */
Subscription::Subscription(const Subscription& other)
{
    snd_seq_port_subscribe_malloc(&m_Info);
    snd_seq_port_subscribe_copy(m_Info, other.m_Info);
}

/**
 * Constructor
 * @param other Pointer to an ALSA subscription object
 */
Subscription::Subscription(snd_seq_port_subscribe_t* other)
{
    snd_seq_port_subscribe_malloc(&m_Info);
    snd_seq_port_subscribe_copy(m_Info, other);
}

/**
 * Constructor
 * @param seq Pointer to a MIDI Client object
 */
Subscription::Subscription(MidiClient* seq)
{
    snd_seq_port_subscribe_malloc(&m_Info);
    CHECK_WARNING(snd_seq_get_port_subscription(seq->getHandle(), m_Info));
}

/**
 * Destructor
 * @return
 */
Subscription::~Subscription()
{
    snd_seq_port_subscribe_free(m_Info);
}

/**
 * Copy the current object
 * @return Pointer to the new object
 */
Subscription*
Subscription::clone()
{
    return new Subscription(m_Info);
}

/**
 * Assignment operator
 * @param other Existing subscription object reference
 * @return This object
 */
Subscription&
Subscription::operator=(const Subscription& other)
{
    snd_seq_port_subscribe_copy(m_Info, other.m_Info);
    return *this;
}

/**
 * Gets the sender address of the subscription (MIDI OUT port)
 * @return Pointer to the sender ALSA address record
 */
const snd_seq_addr_t*
Subscription::getSender()
{
    return snd_seq_port_subscribe_get_sender(m_Info);
}

/**
 * Gets the destination address of the subscription (MIDI IN port)
 * @return Pointer to the destination ALSA address record
 */
const snd_seq_addr_t*
Subscription::getDest()
{
    return snd_seq_port_subscribe_get_dest(m_Info);
}

/**
 * Gets the susbcription's queue number
 * @return Queue number
 */
int
Subscription::getQueue()
{
    return snd_seq_port_subscribe_get_queue(m_Info);
}

/**
 * Gets the subscription's exclusive flag
 * @return Exclusive flag
 */
bool
Subscription::getExclusive()
{
    return (snd_seq_port_subscribe_get_exclusive(m_Info) != 0);
}

/**
 * Gets the susbcription's time-update flag
 * @return Time-update flag
 */
bool
Subscription::getTimeUpdate()
{
    return (snd_seq_port_subscribe_get_time_update(m_Info) != 0);
}

/**
 * Gets the susbcription's time-real (time-stamping) flag
 * @return Time real flag
 */
bool
Subscription::getTimeReal()
{
    return (snd_seq_port_subscribe_get_time_real(m_Info) != 0);
}

/**
 * Sets the Subscription's sender (MIDI OUT) port
 * @param addr Pointer to the sender ALSA address record
 */
void
Subscription::setSender(const snd_seq_addr_t* addr)
{
    snd_seq_port_subscribe_set_sender(m_Info, addr);
}

/**
 * Sets the Subscription's destination (MIDI IN) port
 * @param addr Pointer to the destination ALSA address record
 */
void
Subscription::setDest(const snd_seq_addr_t* addr)
{
    snd_seq_port_subscribe_set_dest(m_Info, addr);
}

/**
 * Sets the Subscription's Queue number
 * @param q Queue number
 */
void
Subscription::setQueue(int q)
{
    snd_seq_port_subscribe_set_queue(m_Info, q);
}

/**
 * Sets the subscription's exclusive flag
 * @param val Exclusive flag
 */
void
Subscription::setExclusive(bool val)
{
    snd_seq_port_subscribe_set_exclusive(m_Info, val?1:0);
}

/**
 * Sets the susbcription's time-update flag
 * @param val Time update flag
 */
void
Subscription::setTimeUpdate(bool val)
{
    snd_seq_port_subscribe_set_time_update(m_Info, val?1:0);
}

/**
 * Sets the subscription's time real (time-stamping) flag
 * @param val Time real flag
 */
void
Subscription::setTimeReal(bool val)
{
    snd_seq_port_subscribe_set_time_real(m_Info, val?1:0);
}

/**
 * Sets the Subscription's sender (MIDI OUT) port
 * @param client Client number
 * @param port Port number
 */
void
Subscription::setSender(unsigned char client, unsigned char port)
{
    snd_seq_addr_t addr;
    addr.client = client;
    addr.port = port;
    setSender(&addr);
}

/**
 * Sets the Subscription's destination (MIDI IN) port
 * @param client Client number
 * @param port Port number
 */
void
Subscription::setDest(unsigned char client, unsigned char port)
{
    snd_seq_addr_t addr;
    addr.client = client;
    addr.port = port;
    setDest(&addr);
}

/**
 * Performs the subscription in the ALSA sequencer subsystem.
 * Neither the sender nor the destination ports need to belong to the
 * same MidiClient instance performing the subscription.
 * @param seq MidiClient instance pointer
 */
void
Subscription::subscribe(MidiClient* seq)
{
    if ((m_Info == NULL) || (seq == NULL) || !(seq->isOpened()))
    {
        return;
    }
    CHECK_WARNING(snd_seq_subscribe_port(seq->getHandle(), m_Info));
}

/**
 * Breaks the subscription in the ALSA sequencer subsystem.
 * Neither the sender nor the destination ports need to belong to the
 * same MidiClient instance breaking the subscription.
 * @param seq MidiClient instance pointer
 */
void
Subscription::unsubscribe(MidiClient* seq)
{
    if ((m_Info == NULL) || (seq == NULL) || !(seq->isOpened()))
    {
        return;
    }
    CHECK_WARNING(snd_seq_unsubscribe_port(seq->getHandle(), m_Info));
}

/**
 * Gets the size of the ALSA subscription object
 * @return Size of the ALSA object
 */
int 
Subscription::getSizeOfInfo() const
{
    return snd_seq_port_subscribe_sizeof();
}

} /* namespace drumstick */

