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
