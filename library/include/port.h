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

 
#ifndef INCLUDED_PORT_H
#define INCLUDED_PORT_H

#include "commons.h"
#include "subscription.h"
#include <QObject>
#include <QList>

namespace ALSA 
{
namespace Sequencer 
{

class MidiClient;

class PortInfo
{
    friend class MidiPort;
    friend class ClientInfo;
    friend class MidiClient;

public:
    PortInfo();
    PortInfo(const PortInfo& other);
    PortInfo(snd_seq_port_info_t* other);
    PortInfo(MidiClient* seq, const int client, const int port);
    virtual ~PortInfo(); 
    PortInfo* clone();
    PortInfo& operator=(const PortInfo& other);
    int getSizeOfInfo() const;
        
    int getClient();
    int getPort();
    QString getClientName() const { return m_ClientName; }
    const snd_seq_addr_t* getAddr();
    QString getName();
    unsigned int getCapability();
    unsigned int getType();
    int getMidiChannels();
    int getMidiVoices();
    int getSynthVoices();
    int getReadUse();
    int getWriteUse();
    int getPortSpecified();
    void setClient(int client);
    void setPort(int port);
    void setAddr(const snd_seq_addr_t* addr);
    void setName( QString const& name );
    void setCapability(unsigned int capability);
    void setType(unsigned int type);
    void setMidiChannels(int channels);
    void setMidiVoices(int voices);
    void setSynthVoices(int voices);
    void setPortSpecified(int val);
    SubscribersList getReadSubscribers() const;
    SubscribersList getWriteSubscribers() const;

    bool getTimestamping();
    bool getTimestampReal();
    int getTimestampQueue();
    void setTimestamping(bool value);
    void setTimestampReal(bool value);
    void setTimestampQueue(int queueId);

protected:    
    void readSubscribers(MidiClient* seq);
    void freeSubscribers();
    void setClientName(QString name) { m_ClientName = name; }

private:
    snd_seq_port_info_t* m_Info;
    QString m_ClientName;
    SubscribersList m_ReadSubscribers;
    SubscribersList m_WriteSubscribers;
};

typedef QList<PortInfo> PortInfoList;

class MidiPort : public QObject
{
    Q_OBJECT
    friend class MidiClient;

public:
    MidiPort( QObject* parent );
    virtual ~MidiPort();

    void attach();
    void detach();
    void subscribe( Subscription* subs );
    void unsubscribe( Subscription* subs );
    void unsubscribeAll();
    void unsubscribeTo( QString const& name );
    void unsubscribeTo( PortInfo* port );
    void unsubscribeTo( const snd_seq_addr_t* addr );
    void unsubscribeFrom( QString const& name );
    void unsubscribeFrom( PortInfo* port );
    void unsubscribeFrom( const snd_seq_addr_t* addr );
    void subscribeTo( PortInfo* port); 
    void subscribeTo( int client, int port ); 
    void subscribeTo( QString const& name ); 
    void subscribeFrom( PortInfo* port ); 
    void subscribeFrom( int client, int port ); 
    void subscribeFrom( QString const& name ); 
    void subscribeFromAnnounce();
    void updateSubscribers();
    SubscriptionsList getSubscriptions() const;
    PortInfoList getReadSubscribers() const;
    PortInfoList getWriteSubscribers() const;
    void updateConnectionsTo(const PortInfoList& desired);
    void updateConnectionsFrom(const PortInfoList& desired);
    bool containsAddress(const snd_seq_addr_t* addr, const PortInfoList& lst);
    
    void setMidiClient( MidiClient* seq );
    void applyPortInfo();
    QString getPortName();
    void setPortName( QString const& newName);
    int getPortId() const;
    unsigned int getCapability();
    void setCapability( unsigned int newValue);
    unsigned int getPortType();
    void setPortType( unsigned int newValue);
    int getMidiChannels();
    void setMidiChannels(int newValue);
    int getMidiVoices();
    void setMidiVoices(int newValue);
    int getSynthVoices();
    void setSynthVoices(int newValue);
    void setAttached(bool state);
    void setAutoAttach(bool state);

signals:
    void subscribed(MidiPort* port, Subscription* subs);
    void midiClientChanged(MidiPort* port, MidiClient* seq);
    void attached(MidiPort* port);
    void detached(MidiPort* port);

protected:
    PortInfo* getPortInfo();
    void freeSubscriptions();

private:
    MidiClient* m_MidiClient;
    PortInfo* m_Info;
    bool m_Attached;
    bool m_AutoAttach;
    SubscriptionsList m_Subscriptions;
};

typedef QList<MidiPort*> MidiPortList;

}
}

#endif //INCLUDED_PORT_H
