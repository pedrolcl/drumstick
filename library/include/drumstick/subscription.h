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

#ifndef DRUMSTICK_SUBSCRIPTION_H
#define DRUMSTICK_SUBSCRIPTION_H

extern "C" {
    #include <alsa/asoundlib.h>
}

#include <QList>
#include "macros.h"

namespace drumstick { namespace ALSA {

/**
 * @file subscription.h
 * Classes managing ALSA sequencer subscriptions
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

class MidiClient;

/**
 * @addtogroup ALSASubs ALSA Sequencer Subscriptions
 * @{
 *
 * @class Subscriber
 * Subscriber container class.
 *
 * This class is used to enumerate the subscribers of a given (root) port.
 */
class DRUMSTICK_ALSA_EXPORT Subscriber
{
    friend class PortInfo;
public:
    Subscriber();
    Subscriber(const Subscriber& other);
    explicit Subscriber(snd_seq_query_subscribe_t* other);
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

private:
    snd_seq_query_subscribe_t* m_Info;

};

/**
 * Subscription management.
 *
 * This class represents a connection between two ports.
 */
class DRUMSTICK_ALSA_EXPORT Subscription
{
public:
    Subscription();
    Subscription(const Subscription& other);
    explicit Subscription(snd_seq_port_subscribe_t* other);
    explicit Subscription(MidiClient* seq);
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

/**
 * List of subscriptions
 */
typedef QList<Subscription> SubscriptionsList;

/**
 * List of subscribers
 */
typedef QList<Subscriber> SubscribersList;

/** @} */

}} /* namespace drumstick::ALSA */

#endif //DRUMSTICK_SUBSCRIPTION_H
