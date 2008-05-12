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

#include <qobject.h>

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
    PortInfo(snd_seq_port_info_t* other);
    virtual ~PortInfo(); 
    PortInfo* clone();
    
    int getClient();
    int getPort();
    const snd_seq_addr_t* getAddr();
    std::string getName();
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
    void setAddr(snd_seq_addr_t* addr);
    void setName(std::string const& name);
    void setName(QString const& name);
    void setCapability(unsigned int capability);
    void setType(unsigned int type);
    void setMidiChannels(int channels);
    void setMidiVoices(int voices);
    void setSynthVoices(int voices);
    void setPortSpecified(int val);

    unsigned int getSubscribersCount();
    Subscriber* getSubscriber(unsigned int j);
    
protected:    
    void readSubscribers(MidiClient* seq);
    void freeSubscribers();
  
    /*PROPERTY  Info: Psnd_seq_port_info_t read FInfo;
     PROPERTY  ClientId: Integer read get_client write set_client;
     PROPERTY  PortId: Integer read get_port write set_port;
     PROPERTY  PortName: String read get_name write set_name;
     PROPERTY  Addr: snd_seq_addr_t read get_addr write set_addr;
     PROPERTY  Capability: TPortCaps read GetCaps write SetCaps;
     PROPERTY  PortType: TPortType read GetType write SetType;
     PROPERTY  MidiChannels: LongInt read get_midi_channels write set_midi_channels;
     PROPERTY  MidiVoices: LongInt read get_midi_voices write set_midi_voices;
     PROPERTY  SynthVoices: LongInt read get_synth_voices write set_synth_voices;
     PROPERTY  PortSpecified: LongInt read get_port_specified write set_port_specified;
     PROPERTY  ReadUse: LongInt read get_read_use;
     PROPERTY  WriteUse: LongInt read get_write_use;
     PROPERTY  SubscribersCount: Integer read GetSubscribersCount;
     PROPERTY  Subscribers[j: Integer]: TAlsaQuerySubscribe read GetSubscriber; */

private:
    snd_seq_port_info_t* m_Info;
    SubscribersVector m_Subscribers;
};

  
class MidiPort : public QObject
{
	Q_OBJECT
	
public:
    MidiPort( QObject* parent, const char* name = 0 );
    virtual ~MidiPort();
    
    void attach();
    void detach();
    void subscribe( Subscription* subs );
    void unsubscribe( Subscription* subs );
    void unsubscribeAll();
    void subscribeTo( PortInfo* port); 
    void subscribeTo( int client, int port); 
    void subscribeTo( std::string const& name); 
    void subscribeTo( QString const& name); 
    void subscribeFrom( PortInfo* port); 
    void subscribeFrom( int client, int port); 
    void subscribeFrom( std::string const& name); 
    void subscribeFrom( QString const& name); 
    void subscribeFromAnnounce();
  
    int getSubscriptionCount();
    Subscription* getSubscription(unsigned int j);
    
    void setMidiClient( MidiClient* seq);
    void applyPortInfo();
    std::string getPortName();
    void setPortName( std::string const& newName);
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
    PortInfo* getPortInfo();
  
signals:
    void subscribed(MidiPort* port, Subscription* subs);
    void midiClientChanged(MidiPort* port, MidiClient* seq);
    void attached(MidiPort* port);
    void detached(MidiPort* port);
    
protected:
    void freeSubscriptions();
	
    /*PROPERTY  SubscriptionCount: Integer read GetSubscriptionCount;
     PROPERTY  Subscription[j: Integer]:TAlsaSubscription read GetSubscription;
     PROPERTY  Info: TAlsaPortInfo read GetPortInfo;
     PROPERTY  Attached: Boolean read FAttached write SetAttached;
     PROPERTY  OnSubscription: TAlsaMidiPortEvent read FOnSubscription write FOnSubscription;
     PROPERTY  OnMidiClientChange: TAlsaMidiPortEvent read FOnMidiClientChange write FOnMidiClientChange;
     PROPERTY  OnAttach: TAlsaMidiPortEvent read FOnAttach write FOnAttach;
     PROPERTY  OnDetach: TAlsaMidiPortEvent read FOnDetach write FOnDetach;
     PROPERTY  AutoAttach: Boolean read FAutoAttach write SetAutoAttach;
     PROPERTY  MidiClient: TAlsaMidiClient read FMidiClient write SetMidiClient;
     PROPERTY  PortName: String read GetPortName write SetPortName;
     PROPERTY  Capability: TPortCaps read GetPortCaps write SetPortCaps;
     PROPERTY  PortType: TPortType read GetPortType write SetPortType;
     PROPERTY  MidiChannels: LongInt read GetMidiChannels write SetMidiChannels;
     PROPERTY  MidiVoices: LongInt read GetMidiVoices write SetMidiVoices;
     PROPERTY  SynthVoices: LongInt read GetSynthVoices write SetSynthVoices; */

private:
    MidiClient* m_MidiClient;
    PortInfo* m_Info;
    bool m_Attached;
    bool m_AutoAttach;
    SubscriptionsVector m_Subscriptions;
};

typedef std::vector<PortInfo*> PortInfoVector;
typedef std::vector<MidiPort*> MidiPortVector;

}
}

#endif //INCLUDED_PORT_H
