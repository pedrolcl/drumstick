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

#include "subscription.h"
#include "client.h"

namespace ALSA
{
namespace Sequencer
{

/******************/
/* Subscriber */
/******************/

Subscriber::Subscriber()
{
    snd_seq_query_subscribe_malloc(&m_Info);
}

Subscriber::Subscriber(const Subscriber& other)
{
    snd_seq_query_subscribe_malloc(&m_Info);
    snd_seq_query_subscribe_copy(m_Info, other.m_Info);
}

Subscriber::Subscriber(snd_seq_query_subscribe_t* other)
{
    snd_seq_query_subscribe_malloc(&m_Info);
    snd_seq_query_subscribe_copy(m_Info, other);
}

Subscriber::~Subscriber()
{
    snd_seq_query_subscribe_free(m_Info);
}

Subscriber* Subscriber::clone()
{
    return new Subscriber(m_Info);
}

Subscriber& Subscriber::operator=(const Subscriber& other)
{
    snd_seq_query_subscribe_copy(m_Info, other.m_Info);
    return *this;
}

int
Subscriber::getClient()
{
    return snd_seq_query_subscribe_get_client(m_Info);
}

int
Subscriber::getPort()
{
    return snd_seq_query_subscribe_get_port(m_Info);
}

const snd_seq_addr_t*
Subscriber::getRoot()
{
    return snd_seq_query_subscribe_get_root(m_Info);
}

snd_seq_query_subs_type_t
Subscriber::getType()
{
    return snd_seq_query_subscribe_get_type(m_Info);
}

int
Subscriber::getIndex()
{
    return snd_seq_query_subscribe_get_index(m_Info);
}

int
Subscriber::getNumSubs()
{
    return snd_seq_query_subscribe_get_num_subs(m_Info);
}

const snd_seq_addr_t*
Subscriber::getAddr()
{
    return snd_seq_query_subscribe_get_addr(m_Info);
}

int
Subscriber::getQueue()
{
    return snd_seq_query_subscribe_get_queue(m_Info);
}

bool
Subscriber::getExclusive()
{
    return (snd_seq_query_subscribe_get_exclusive(m_Info) != 0);
}

bool
Subscriber::getTimeUpdate()
{
    return (snd_seq_query_subscribe_get_time_update(m_Info) != 0);
}

bool
Subscriber::getTimeReal()
{
    return (snd_seq_query_subscribe_get_time_real(m_Info) != 0);
}

void
Subscriber::setClient(int client)
{
    snd_seq_query_subscribe_set_client(m_Info, client);
}

void
Subscriber::setPort(int port)
{
    snd_seq_query_subscribe_set_port(m_Info, port);
}

void
Subscriber::setRoot(snd_seq_addr_t* addr)
{
    snd_seq_query_subscribe_set_root(m_Info, addr);
}

void
Subscriber::setType(snd_seq_query_subs_type_t type)
{
    snd_seq_query_subscribe_set_type(m_Info, type);
}

void
Subscriber::setIndex(int index)
{
    snd_seq_query_subscribe_set_index(m_Info, index);
}

/****************/
/* Subscription */
/****************/

Subscription::Subscription()
{
    snd_seq_port_subscribe_malloc(&m_Info);
}

Subscription::Subscription(const Subscription& other)
{
    snd_seq_port_subscribe_malloc(&m_Info);
    snd_seq_port_subscribe_copy(m_Info, other.m_Info);
}

Subscription::Subscription(snd_seq_port_subscribe_t* other)
{
    snd_seq_port_subscribe_malloc(&m_Info);
    snd_seq_port_subscribe_copy(m_Info, other);
}

Subscription::~Subscription()
{
    snd_seq_port_subscribe_free(m_Info);
}

Subscription*
Subscription::clone()
{
    return new Subscription(m_Info);
}

Subscription&
Subscription::operator=(const Subscription& other)
{
    snd_seq_port_subscribe_copy(m_Info, other.m_Info);
    return *this;
}

const snd_seq_addr_t*
Subscription::getSender()
{
    return snd_seq_port_subscribe_get_sender(m_Info);
}

const snd_seq_addr_t*
Subscription::getDest()
{
    return snd_seq_port_subscribe_get_dest(m_Info);
}

int
Subscription::getQueue()
{
    return snd_seq_port_subscribe_get_queue(m_Info);
}

bool
Subscription::getExclusive()
{
    return (snd_seq_port_subscribe_get_exclusive(m_Info) != 0);
}

bool
Subscription::getTimeUpdate()
{
    return (snd_seq_port_subscribe_get_time_update(m_Info) != 0);
}

bool
Subscription::getTimeReal()
{
    return (snd_seq_port_subscribe_get_time_real(m_Info) != 0);
}

void
Subscription::setSender(const snd_seq_addr_t* addr)
{
    snd_seq_port_subscribe_set_sender(m_Info, addr);
}

void
Subscription::setDest(const snd_seq_addr_t* addr)
{
    snd_seq_port_subscribe_set_dest(m_Info, addr);
}

void
Subscription::setQueue(int q)
{
    snd_seq_port_subscribe_set_queue(m_Info, q);
}

void
Subscription::setExclusive(bool val)
{
    snd_seq_port_subscribe_set_exclusive(m_Info, val?1:0);
}

void
Subscription::setTimeUpdate(bool val)
{
    snd_seq_port_subscribe_set_time_update(m_Info, val?1:0);
}

void
Subscription::setTimeReal(bool val)
{
    snd_seq_port_subscribe_set_time_real(m_Info, val?1:0);
}

void
Subscription::setSender(unsigned char client, unsigned char port)
{
    snd_seq_addr_t addr;
    addr.client = client;
    addr.port = port;
    setSender(&addr);
}

void
Subscription::setDest(unsigned char client, unsigned char port)
{
    snd_seq_addr_t addr;
    addr.client = client;
    addr.port = port;
    setDest(&addr);
}

void
Subscription::subscribe(MidiClient* seq)
{
    if ((m_Info == NULL) || (seq == NULL) || !(seq->isOpened()))
    {
        return;
    }
    CHECK_EXCEPT(snd_seq_subscribe_port(seq->getHandle(), m_Info));
}

void
Subscription::unsubscribe(MidiClient* seq)
{
    if ((m_Info == NULL) || (seq == NULL) || !(seq->isOpened()))
    {
        return;
    }
    CHECK_EXCEPT(snd_seq_unsubscribe_port(seq->getHandle(), m_Info));
}

}
}
