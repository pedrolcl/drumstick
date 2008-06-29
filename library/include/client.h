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
#include <QObject>
#include <QList>

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
	ClientInfo(const ClientInfo& other);
	ClientInfo(snd_seq_client_info_t* other); 
	virtual ~ClientInfo();
    ClientInfo* clone();
    ClientInfo& operator=(const ClientInfo& other);

    int getClientId();
    snd_seq_client_type_t getClientType();
    QString getName();
    bool getBroadcastFilter();
    bool getErrorBounce();
    int getNumPorts();
    int getEventLost();
    void setClient(int client);
    void setName(QString name);
    void setBroadcastFilter(bool val);
    void setErrorBounce(bool val);
    const unsigned char* getEventFilter();
    void setEventFilter(unsigned char* filter);
    
    unsigned int getPortInfoCount();
    PortInfo* getPortInfo(int j);

protected:    
    void readPorts(MidiClient* seq);
    void freePorts();
  
private:
    snd_seq_client_info_t* m_Info;
    PortInfoList m_Ports;
};

typedef QList<ClientInfo> ClientInfoList;

class MidiClient : public QObject
{
	Q_OBJECT
	friend class SequencerInputThread;
	
public:
	MidiClient( QObject* parent );
	virtual ~MidiClient();
	
    void open();
    void close();
    void startEvents();
    void stopEvents();
    MidiPort* createPort();
    MidiQueue* createQueue(); 
    MidiQueue* createQueue(QString const& name);
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
    QString getDeviceName() { return m_DeviceName; }
    void setDeviceName( QString const& newName);
    int getOpenMode() { return m_OpenMode; }
    void setOpenMode(int newMode);
    bool getBlockMode() { return m_BlockMode; }
    void setBlockMode(bool newValue);
    QString getClientName();
    void setClientName( QString const& newName);
    bool getBroadcastFilter();
    void setBroadcastFilter(bool newValue);
    bool getErrorBounce();
    void setErrorBounce(bool newValue);
    
    ClientInfo* getThisClientInfo();
    void setThisClientInfo(ClientInfo* val);
    int getPortCount();
    MidiPort* getPort(int j);
    int getClientInfoCount();
    ClientInfo* getClientInfo(int j);
    PortInfoList getAvailableInputs();
    PortInfoList getAvailableOutputs();

protected:    
    void doEvents();
    void applyClientInfo();
    void readClients();
    void freeClients();
    void updateAvailablePorts();
    PortInfoList filterPorts(unsigned int filter);

private:
    snd_seq_t* m_SeqHandle;
    QString m_DeviceName;
    
    bool m_BlockMode;
    bool m_NeedRefreshClientList;
    int  m_OpenMode;
    
    SequencerInputThread* m_Thread;
    ClientInfo* m_Info;
    MidiQueue* m_Queue;

    ClientInfoList m_ClientList;
    MidiPortList m_Ports;
    PortInfoList m_OutputsAvail;
    PortInfoList m_InputsAvail;
};

}
}

#endif //INCLUDED_CLIENT_H
