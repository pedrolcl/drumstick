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

 
#ifndef INCLUDED_SUBSCRIPTION_H
#define INCLUDED_SUBSCRIPTION_H

#include "commons.h"
#include <QList>

namespace ALSA 
{
namespace Sequencer 
{

class MidiClient;

class Subscriber
{
    friend class PortInfo;

public:
    Subscriber();
    Subscriber(const Subscriber& other);
    Subscriber(snd_seq_query_subscribe_t* other);
    virtual ~Subscriber();
    Subscriber* clone();
    int getSizeOfInfo() const;
    
    int getClient();
    int getPort();
    const snd_seq_addr_t* getRoot();
    snd_seq_query_subs_type_t getType();
    int getIndex();
    int getNumSubs();
    const snd_seq_addr_t* getAddr();
    int getQueue();
    bool getExclusive();
    bool getTimeUpdate();
    bool getTimeReal();
    void setClient(int client);
    void setPort(int port);
    void setRoot(snd_seq_addr_t* addr);
    void setType(snd_seq_query_subs_type_t type);
    void setIndex(int index);
    Subscriber& operator=(const Subscriber& other);
    bool operator==(Subscriber& other);

private:
    snd_seq_query_subscribe_t* m_Info;

};

class Subscription
{
public:
    Subscription();
    Subscription(const Subscription& other);
    Subscription(snd_seq_port_subscribe_t* other);
    Subscription(MidiClient* seq);
    virtual ~Subscription();
    Subscription* clone();
    int getSizeOfInfo() const;
    
    void setSender(unsigned char client, unsigned char port);
    void setDest(unsigned char client, unsigned char port);
    void subscribe(MidiClient* seq);
    void unsubscribe(MidiClient* seq);

    const snd_seq_addr_t* getSender();
    const snd_seq_addr_t* getDest();
    int getQueue();
    bool getExclusive();
    bool getTimeUpdate();
    bool getTimeReal();
    void setSender(const snd_seq_addr_t* addr);
    void setDest(const snd_seq_addr_t* addr);
    void setQueue(int queue);
    void setExclusive(bool val);
    void setTimeUpdate(bool val);
    void setTimeReal(bool val);
    Subscription& operator=(const Subscription& other);

private:
    snd_seq_port_subscribe_t* m_Info;
};

typedef QList<Subscription> SubscriptionsList;
typedef QList<Subscriber> SubscribersList;

}
}

#endif //INCLUDED_SUBSCRIPTION_H
