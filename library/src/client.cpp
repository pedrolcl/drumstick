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

#include "commons.h"
#include "client.h"
#include "queue.h"
#include "event.h"
#include "qmidithread.h"
#include "port.h"
#include <qthread.h>
#include <qapplication.h>

namespace ALSA 
{
namespace Sequencer 
{

/**************/
/* MidiClient */
/**************/

MidiClient::MidiClient( QObject* parent, const char* name ) : 
	QObject(parent, name),
	m_SeqHandle(0),
	m_DeviceName("default"),
	m_BlockMode(false),
	m_NeedRefreshClientList(true),
    m_OpenMode(SND_SEQ_OPEN_DUPLEX),
    m_Thread(NULL),
    m_Info(NULL),
    m_Queue(NULL)
{
    m_Info = new ClientInfo();
}

MidiClient::~MidiClient()
{
    detachAllPorts();
    if (m_Queue != NULL)
        delete m_Queue;
    close();
    freeClients();
    if (m_Thread != NULL)
    	delete m_Thread;
    if (m_Info != NULL)
    	delete m_Info;
}

void 
MidiClient::open()
{
    int blockMode = 0; 
    if (!m_BlockMode) {
        blockMode = SND_SEQ_NONBLOCK;
    }
    CHECK_ERROR(snd_seq_open(&m_SeqHandle, m_DeviceName.c_str(), m_OpenMode, blockMode));
    snd_seq_get_client_info(m_SeqHandle, m_Info->m_Info);
}

void 
MidiClient::close()
{
    if (m_SeqHandle != NULL) {
        stopEvents();
        CHECK_ERROR(snd_seq_close(m_SeqHandle));
        m_SeqHandle = NULL;
    }
}

size_t 
MidiClient::getOutputBufferSize()
{
    return snd_seq_get_output_buffer_size(m_SeqHandle);
}

void 
MidiClient::setOutputBufferSize(size_t newSize)
{
    if (getOutputBufferSize() != newSize) {
        CHECK_ERROR(snd_seq_set_output_buffer_size(m_SeqHandle, newSize));
    }
}

size_t
MidiClient::getInputBufferSize()
{
    return snd_seq_get_input_buffer_size(m_SeqHandle);
}

void 
MidiClient::setInputBufferSize(size_t newSize)
{
    if (getInputBufferSize() != newSize) {
    	CHECK_ERROR(snd_seq_set_input_buffer_size(m_SeqHandle, newSize));
    }
}

void 
MidiClient::setDeviceName( std::string const& newName)
{
    if ((m_DeviceName != newName) && (m_SeqHandle == NULL)) {
        m_DeviceName = newName;
    }
}

void 
MidiClient::setOpenMode(int newMode)
{
    if ((m_OpenMode != newMode) && (m_SeqHandle == NULL)) {
        m_OpenMode = newMode;
    }
}

void
MidiClient::setBlockMode(bool newValue)
{
    if (m_BlockMode != newValue) {
        m_BlockMode = newValue;
        if (m_SeqHandle != NULL) {
        	snd_seq_nonblock(m_SeqHandle, m_BlockMode ? 0 : 1);
        }
    }
}

int 
MidiClient::getClientId()
{
    return snd_seq_client_id(m_SeqHandle);
}

snd_seq_type_t 
MidiClient::getSequencerType()
{
    return snd_seq_type(m_SeqHandle);
}

void 
MidiClient::doEvents()
{
    do {
        int err = 0;
        snd_seq_event_t* evp = NULL;
        SequencerEvent* event = NULL;
        err = snd_seq_event_input(m_SeqHandle, &evp);
        if ((err >= 0) && (evp != NULL)) {
            switch (evp->type) {
            
                case SND_SEQ_EVENT_NOTE:
                    event = new NoteEvent(evp);
                    break;
                    
                case SND_SEQ_EVENT_NOTEON:
                    event = new NoteOnEvent(evp);
                    break;
                    
                case SND_SEQ_EVENT_NOTEOFF:
                    event = new NoteOffEvent(evp);
                    break;
                
                case SND_SEQ_EVENT_KEYPRESS: 
                    event = new KeyPressEvent(evp);
                    break;
                
                case SND_SEQ_EVENT_CONTROLLER: 
                case SND_SEQ_EVENT_CONTROL14:
                case SND_SEQ_EVENT_REGPARAM:
                case SND_SEQ_EVENT_NONREGPARAM:
                	event = new ControllerEvent(evp);
                    break;
                
                case SND_SEQ_EVENT_PGMCHANGE: 
                    event = new ProgramChangeEvent(evp);
                    break;
                
                case SND_SEQ_EVENT_CHANPRESS: 
                    event = new ChanPressEvent(evp);
                    break;
                
                case SND_SEQ_EVENT_PITCHBEND: 
                    event = new PitchBendEvent(evp);
                    break;
                
                case SND_SEQ_EVENT_SYSEX: 
                    event = new SysExEvent(evp);
                    break;
                
                case SND_SEQ_EVENT_PORT_SUBSCRIBED:
                case SND_SEQ_EVENT_PORT_UNSUBSCRIBED:
                	event = new SubscriptionEvent(evp);
                	break;
                	
                case SND_SEQ_EVENT_PORT_CHANGE:
                case SND_SEQ_EVENT_PORT_EXIT:
                case SND_SEQ_EVENT_PORT_START:
                	event = new PortEvent(evp);
                	m_NeedRefreshClientList = true;
                	break;
                	
                case SND_SEQ_EVENT_CLIENT_CHANGE:
                case SND_SEQ_EVENT_CLIENT_EXIT:
                case SND_SEQ_EVENT_CLIENT_START:
                	event = new ClientEvent(evp);
                	m_NeedRefreshClientList = true;
                	break;
                	
            	case SND_SEQ_EVENT_SONGPOS:
            	case SND_SEQ_EVENT_SONGSEL:
            	case SND_SEQ_EVENT_QFRAME:
            	case SND_SEQ_EVENT_TIMESIGN:
            	case SND_SEQ_EVENT_KEYSIGN:
            		event = new ValueEvent(evp);
            		break;
                	
            	case SND_SEQ_EVENT_SETPOS_TICK:
            	case SND_SEQ_EVENT_SETPOS_TIME:
            	case SND_SEQ_EVENT_QUEUE_SKEW:
            		event = new QueueControlEvent(evp);
            		break;
            		
            	case SND_SEQ_EVENT_TEMPO:
            		event = new TempoEvent(evp);
            		break;
            		
                default: 
                    event = new SequencerEvent(evp);
                    break;
            }
            qApp->sendEvent(parent(), event);
        }
    }
    while (snd_seq_event_input_pending(m_SeqHandle, 0) > 0);
}

void 
MidiClient::startEvents()
{
	if (m_Thread == NULL) {
		m_Thread = new SequencerInputThread(this, POLLIN, 500);
		m_Thread->start();
	}
}

void 
MidiClient::stopEvents()
{
    if (m_Thread != NULL) {
    	m_Thread->stop();
    	int counter = 0;
    	while (!m_Thread->wait(500) && (counter < 10)) {
        	counter++;
    	}
        if (!m_Thread->finished()) {
        	m_Thread->terminate();
        }
        m_Thread = NULL;
    }
}

void 
MidiClient::readClients()
{
    ClientInfo *cInfo1, *cInfo2; 
    freeClients();
    cInfo1 = new ClientInfo();
    cInfo1->setClient(-1);
    while (snd_seq_query_next_client(m_SeqHandle, cInfo1->m_Info) >= 0) {
        cInfo2 = cInfo1->clone();
        cInfo2->readPorts(this);
        m_ClientList.push_back(cInfo2);
    }
    delete cInfo1;
    m_NeedRefreshClientList = false;
}

void
MidiClient::freeClients()
{
	ClientInfoVector::iterator it;
	for(it = m_ClientList.begin(); it != m_ClientList.end(); ++it) {
		delete (*it);
	}
    m_ClientList.clear();
}

int 
MidiClient::getClientInfoCount()
{
    return m_ClientList.size();
}

ClientInfo* 
MidiClient::getClientInfo(unsigned int j)
{
    if ((j >= 0) && (j < m_ClientList.size())) {
        return m_ClientList[j];
    } 
    return NULL;
}

ClientInfo* 
MidiClient::getThisClientInfo()
{
    snd_seq_get_client_info(m_SeqHandle, m_Info->m_Info);
    return m_Info;
}

void 
MidiClient::setThisClientInfo(ClientInfo* val)
{
    if (m_Info != val) {
        m_Info = val;
        snd_seq_set_client_info(m_SeqHandle, m_Info->m_Info);
    }
}

void 
MidiClient::applyClientInfo()
{
    if (m_SeqHandle != NULL) {
        snd_seq_set_client_info(m_SeqHandle, m_Info->m_Info);
    }
}

std::string 
MidiClient::getClientName()
{
    return m_Info->getName();
}

void 
MidiClient::setClientName(std::string const& newName)
{
    if (newName != m_Info->getName()) {
        m_Info->setName(newName);
        applyClientInfo();
    }
}

int 
MidiClient::getPortCount()
{
    return m_Ports.size();
}

MidiPort* 
MidiClient::getPort(unsigned int j)
{
    if (j < m_Ports.size()) {
        return m_Ports[j];
    }
    return NULL;
}

MidiPort* 
MidiClient::createPort()
{
	MidiPort* port = new MidiPort(this);
    port->setMidiClient(this);
    return port;
}

void 
MidiClient::portAttach(MidiPort* port)
{
    if (m_SeqHandle != NULL) {
    	CHECK_ERROR(snd_seq_create_port(m_SeqHandle, port->getPortInfo()->m_Info));
    	m_Ports.push_back(port);
    }
}

void 
MidiClient::portDetach(MidiPort* port)
{
    if (m_SeqHandle != NULL) {
		if(port->getPortInfo()->getClient() == getClientId())
		{
			return;
		}
	    CHECK_ERROR(snd_seq_delete_port(m_SeqHandle, port->getPortInfo()->getPort()));
	    port->setMidiClient(NULL);
	    
	    MidiPortVector::iterator it;
	    for(it = m_Ports.begin(); it != m_Ports.end(); ++it)
	    {
	    	if ((*it)->getPortInfo()->getPort() == port->getPortInfo()->getPort())
	    	{
	    		m_Ports.erase(it);
	    		break;
	    	}
	    }
    }
}

void MidiClient::detachAllPorts()
{
	if (m_SeqHandle != NULL) {
		MidiPortVector::iterator it;
		for (it = m_Ports.begin(); it != m_Ports.end(); ++it) {
			CHECK_ERROR(snd_seq_delete_port(m_SeqHandle, (*it)->getPortInfo()->getPort()));
			(*it)->setMidiClient(NULL);
			m_Ports.erase(it);
		}
	}
}

void 
MidiClient::addEventFilter(int evtype)
{
    snd_seq_set_client_event_filter(m_SeqHandle, evtype);
}

bool 
MidiClient::getBroadcastFilter()
{
    return m_Info->getBroadcastFilter();
}

void 
MidiClient::setBroadcastFilter(bool newValue)
{
    m_Info->setBroadcastFilter(newValue);
    applyClientInfo();
}

bool 
MidiClient::getErrorBounce()
{
    return m_Info->getErrorBounce();
}

void 
MidiClient::setErrorBounce(bool newValue)
{
    m_Info->setErrorBounce(newValue);
    applyClientInfo();
}

void 
MidiClient::output(SequencerEvent* ev)
{
    int npfds;
    pollfd* pfds;

    npfds = snd_seq_poll_descriptors_count(m_SeqHandle, POLLOUT);
    pfds = (pollfd*) alloca(npfds * sizeof(pollfd));
    snd_seq_poll_descriptors(m_SeqHandle, pfds, npfds, POLLOUT);
    while (snd_seq_event_output(m_SeqHandle, ev->getEvent()) < 0) {
        CHECK_ERROR(poll(pfds, npfds, -1));
    }
}

void 
MidiClient::outputDirect(SequencerEvent* ev)
{
    int npfds;
    pollfd* pfds;

    npfds = snd_seq_poll_descriptors_count(m_SeqHandle, POLLOUT);
    pfds = (pollfd*) alloca(npfds * sizeof(pollfd));
    snd_seq_poll_descriptors(m_SeqHandle, pfds, npfds, POLLOUT);
    while (snd_seq_event_output_direct(m_SeqHandle, ev->getEvent()) < 0) {
        CHECK_ERROR( poll(pfds, npfds, -1) );
    }
}

void 
MidiClient::outputBuffer(SequencerEvent* ev)
{
    int npfds;
    pollfd* pfds;

    npfds = snd_seq_poll_descriptors_count(m_SeqHandle, POLLOUT);
    pfds = (pollfd*) alloca(npfds * sizeof(pollfd));
    snd_seq_poll_descriptors(m_SeqHandle, pfds, npfds, POLLOUT);
    while (snd_seq_event_output_buffer(m_SeqHandle, ev->getEvent()) < 0) {
        CHECK_ERROR( poll(pfds, npfds, -1) );
    }
}

void 
MidiClient::drainOutput(int timeout)
{
    int npfds;
    pollfd* pfds;

    npfds = snd_seq_poll_descriptors_count(m_SeqHandle, POLLOUT);
    pfds = (pollfd*) alloca(npfds * sizeof(pollfd));
    snd_seq_poll_descriptors(m_SeqHandle, pfds, npfds, POLLOUT);
    while (snd_seq_drain_output(m_SeqHandle) < 0) {
        CHECK_ERROR( poll(pfds, npfds, timeout) );
    }
}

void 
MidiClient::drainOutput()
{
    drainOutput(-1);
}

void 
MidiClient::synchronizeOutput()
{
    CHECK_ERROR(snd_seq_sync_output_queue(m_SeqHandle));
}

MidiQueue* 
MidiClient::getQueue()
{
    if (m_Queue == NULL) {
        createQueue();
    }
    return m_Queue;
}

MidiQueue* 
MidiClient::createQueue()
{
    if (m_Queue != NULL) {
        delete m_Queue;
    }
    m_Queue = new MidiQueue(this);
    return m_Queue;
}

MidiQueue* 
MidiClient::createQueue(std::string const& queueName )
{
    if (m_Queue != NULL) {
        delete m_Queue;
    }
    m_Queue = new MidiQueue(this, queueName);
    return m_Queue;
}

PortInfoVector
MidiClient::filterPorts(unsigned int filter)
{
	PortInfoVector result;
    ClientInfoVector::iterator it;
    unsigned int j;
  
    if (m_NeedRefreshClientList)
    	readClients();
    
    for (it = m_ClientList.begin(); it != m_ClientList.end(); ++it) {
    	ClientInfo* ci = (*it);
        for (j=0; j < ci->getPortInfoCount(); ++j) {
        	PortInfo* pi = ci->getPortInfo(j);
        	unsigned int cap = pi->getCapability();
            if ( ((filter & cap) != 0) && 
            	 ((SND_SEQ_PORT_CAP_NO_EXPORT & cap) == 0) && 
            	 (ci->getClientId() != SND_SEQ_CLIENT_SYSTEM) ) {
                result.push_back(pi);
            }
        }
    }
    return result;
}

void 
MidiClient::updateAvailablePorts()
{
    m_InputsAvail.clear();
    m_OutputsAvail.clear();
    m_InputsAvail = filterPorts( SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ );
    m_OutputsAvail = filterPorts( SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE );
}

PortInfoVector 
MidiClient::getAvailableInputs()
{
    if (m_NeedRefreshClientList || m_InputsAvail.empty()) {
        updateAvailablePorts();
    }
    return m_InputsAvail;
}

PortInfoVector 
MidiClient::getAvailableOutputs()
{
    if (m_NeedRefreshClientList || m_OutputsAvail.empty()) {
        updateAvailablePorts();
    }
    return m_OutputsAvail;
}

/**************/
/* ClientInfo */
/**************/

ClientInfo::ClientInfo()
{
    snd_seq_client_info_malloc(&m_Info);
}

ClientInfo::ClientInfo(snd_seq_client_info_t* other)
{
    snd_seq_client_info_malloc(&m_Info);
    snd_seq_client_info_copy(m_Info, other);
}

ClientInfo::~ClientInfo()
{
    freePorts();
    snd_seq_client_info_free(m_Info);
}

ClientInfo*
ClientInfo::clone()
{
	return new ClientInfo(m_Info);
}

int 
ClientInfo::getClientId()
{
    return snd_seq_client_info_get_client(m_Info);
}

snd_seq_client_type_t 
ClientInfo::getClientType()
{
    return snd_seq_client_info_get_type(m_Info);
}

std::string 
ClientInfo::getName()
{
    return std::string(snd_seq_client_info_get_name(m_Info));
}

bool 
ClientInfo::getBroadcastFilter()
{
    return (snd_seq_client_info_get_broadcast_filter(m_Info) != 0);
}

bool 
ClientInfo::getErrorBounce()
{
    return (snd_seq_client_info_get_error_bounce(m_Info) != 0);
}

const unsigned char* 
ClientInfo::getEventFilter()
{
    return snd_seq_client_info_get_event_filter(m_Info);
}

int 
ClientInfo::getNumPorts()
{
    return snd_seq_client_info_get_num_ports(m_Info);
}

int 
ClientInfo::getEventLost()
{
    return snd_seq_client_info_get_event_lost(m_Info);
}

void 
ClientInfo::setClient(int client)
{
    snd_seq_client_info_set_client(m_Info, client);
}

void 
ClientInfo::setName(std::string name)
{
    snd_seq_client_info_set_name(m_Info, name.c_str());
}

void 
ClientInfo::setBroadcastFilter(bool val)
{
	snd_seq_client_info_set_broadcast_filter(m_Info, val ? 1 : 0);
}

void 
ClientInfo::setErrorBounce(bool val)
{
	snd_seq_client_info_set_error_bounce(m_Info, val ? 1 : 0);
}

void
ClientInfo::setEventFilter(unsigned char *filter)
{
    snd_seq_client_info_set_event_filter(m_Info, filter);
}

void 
ClientInfo::readPorts(MidiClient* seq)
{
    PortInfo *info1, *info2;
    freePorts();
    info1 = new PortInfo();
    info1->setClient(getClientId());
    info1->setPort(-1);
    while (snd_seq_query_next_port(seq->getHandle(), info1->m_Info) >= 0) {
        info2 = info1->clone();
        info2->readSubscribers(seq);
        m_Ports.push_back(info2);
    }
    delete info1;
}

void
ClientInfo::freePorts()
{
    PortInfoVector::iterator it;
    for(it = m_Ports.begin(); it != m_Ports.end(); ++it) {
    	delete (*it);
    }
    m_Ports.clear();
}

unsigned int 
ClientInfo::getPortInfoCount()
{
    return m_Ports.size();
}

PortInfo*
ClientInfo::getPortInfo(unsigned int j)
{
    if (j < m_Ports.size()) {
        return m_Ports[j];
    }
    return NULL;
}

}
}

