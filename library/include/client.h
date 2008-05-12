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
 
#ifndef INCLUDED_CLIENT_H
#define INCLUDED_CLIENT_H

#include "commons.h"
#include "port.h"
#include <vector>
#include <qobject.h>

namespace ALSA 
{
namespace Sequencer 
{
  
class MidiQueue;
class MidiClient;
class SequencerEvent;
class SequencerInputThread;

class ClientInfo
{
	friend class MidiClient;
	
public:
	ClientInfo();
	ClientInfo(snd_seq_client_info_t* other); 
	virtual ~ClientInfo();
    ClientInfo* clone();

    int getClientId();
    snd_seq_client_type_t getClientType();
    std::string getName();
    bool getBroadcastFilter();
    bool getErrorBounce();
    int getNumPorts();
    int getEventLost();
    void setClient(int client);
    void setName(std::string name);
    void setBroadcastFilter(bool val);
    void setErrorBounce(bool val);
    const unsigned char* getEventFilter();
    void setEventFilter(unsigned char* filter);
    
    unsigned int getPortInfoCount();
    PortInfo* getPortInfo(unsigned int j);

protected:    
    void readPorts(MidiClient* seq);
    void freePorts();
  
  /* PROPERTY  Info: Psnd_seq_client_info_t read FInfo;
     PROPERTY  ClientId: Integer read get_client write set_client;
     PROPERTY  ClientType: snd_seq_client_type_t read get_type;
     PROPERTY  Name: String read get_name write set_name;
     PROPERTY  BroadcastFilter: Boolean read get_broadcast_filter write set_broadcast_filter;
     PROPERTY  ErrorBounce: Boolean read get_error_bounce write set_error_bounce;
     property  EventFilter: PByte read get_event_filter write set_event_filter;
     PROPERTY  PortCount: LongInt read get_num_ports;
     PROPERTY  EventLost: LongInt read get_event_lost;
     PROPERTY  PortInfoCount: Integer read GetPortInfoCount;
     PROPERTY  PortInfo[j:Integer]: TAlsaPortInfo read GetPortInfo; */

private:
    snd_seq_client_info_t* m_Info;
    PortInfoVector m_Ports;
};

typedef std::vector<ClientInfo*> ClientInfoVector;

class MidiClient : public QObject
{
	Q_OBJECT
	friend class SequencerInputThread;
	
public:
	MidiClient( QObject* parent, const char* name = 0 );
	virtual ~MidiClient();
	
    void open();
    void close();
    void startEvents();
    void stopEvents();
    MidiPort* createPort();
    MidiQueue* createQueue(); 
    MidiQueue* createQueue(std::string const& name);
    MidiQueue* getQueue();
    void portAttach(MidiPort* port);
    void portDetach(MidiPort* port);
    void detachAllPorts();
    void addEventFilter(int evtype);
    void output(SequencerEvent* ev);
    void outputDirect(SequencerEvent* ev);
    void outputBuffer(SequencerEvent* ev);
    void drainOutput(); 
    void drainOutput(int timeout); 
    void synchronizeOutput();
  
    int getClientId();
    snd_seq_type_t getSequencerType();
    snd_seq_t* getHandle() { return m_SeqHandle; }
    bool isOpened() { return (m_SeqHandle != NULL); }

    size_t getOutputBufferSize();
    void setOutputBufferSize(size_t newSize);
    size_t getInputBufferSize();
    void setInputBufferSize(size_t newSize);
    std::string getDeviceName() { return m_DeviceName; }
    void setDeviceName( std::string const& newName);
    int getOpenMode() { return m_OpenMode; }
    void setOpenMode(int newMode);
    bool getBlockMode() { return m_BlockMode; }
    void setBlockMode(bool newValue);
    std::string getClientName();
    void setClientName( std::string const& newName);
    bool getBroadcastFilter();
    void setBroadcastFilter(bool newValue);
    bool getErrorBounce();
    void setErrorBounce(bool newValue);
    
    ClientInfo* getThisClientInfo();
    void setThisClientInfo(ClientInfo* val);
    int getPortCount();
    MidiPort* getPort(unsigned int j);
    int getClientInfoCount();
    ClientInfo* getClientInfo(unsigned int j);
    PortInfoVector getAvailableInputs();
    PortInfoVector getAvailableOutputs();

protected:    
    void doEvents();
    void applyClientInfo();
    void readClients();
    void freeClients();
    void updateAvailablePorts();
    PortInfoVector filterPorts(unsigned int filter);

  /* PROPERTY  OutputBufferSize: LongInt read GetOutputBufferSize write SetOutputBufferSize;
     PROPERTY  InputBufferSize: LongInt read GetInputBufferSize write SetInputBufferSize;
     PROPERTY  SeqType: LongInt read GetType;
     PROPERTY  ClientId: LongInt read GetClientId;
     PROPERTY  SeqHandle:Psnd_seq_t read FSeqHandle;
     PROPERTY  ClientInfoCount: Integer read GetClientInfoCount;
     PROPERTY  ClientInfo[j:Integer]: TAlsaClientInfo read GetClientInfo;
     PROPERTY  Client: TAlsaClientInfo read GetThisClient write SetThisClient;
     PROPERTY  PortCount: Integer read GetPortCount;
     PROPERTY  Port[j: Integer]: TAlsaMidiPort read GetPort;
     PROPERTY  MidiQueue: TAlsaQueue read GetQueue;
     PROPERTY  OnEvent: TAlsaNotifyEvent read FOnEvent write FOnEvent;
     PROPERTY  OpenMode: TAlsaOpenMode read FMode write SetMode;
     PROPERTY  SeqName:String read FName write SetSeqName;
     PROPERTY  BlockMode: Boolean read FBlock write SetBlock;
     PROPERTY  Enabled: Boolean read FEnabled write SetEnabled;
     PROPERTY  ClientName: String read GetClientName write SetClientName;
     PROPERTY  BroadcastFilter: Boolean read GetBroadcastFilter write SetBroadcastFilter;
     PROPERTY  ErrorBounce: Boolean read GetErrorBounce write SetErrorBounce;  */

private:
    snd_seq_t* m_SeqHandle;
    std::string m_DeviceName;
    
    bool m_BlockMode;
    bool m_NeedRefreshClientList;
    int  m_OpenMode;
    
    SequencerInputThread* m_Thread;
    ClientInfo* m_Info;
    MidiQueue* m_Queue;

    ClientInfoVector m_ClientList;
    MidiPortVector m_Ports;
    PortInfoVector m_OutputsAvail;
    PortInfoVector m_InputsAvail;
};

}
}

#endif //INCLUDED_CLIENT_H
