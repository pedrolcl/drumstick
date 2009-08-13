/*
    MIDI Sequencer C++ library
    Copyright (C) 2006-2009, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

/*!
\mainpage aseqmm documentation
\author Copyright &copy; 2009 Pedro López-Cabanillas
\version 0.0.3
\date 2009-08-12

This document is licensed under the Creative Commons Attribution-Share Alike 3.0 Unported License.
To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/

\section Abstract

\section Contents Table of Contents

- \ref Disclaimer
- \ref Introduction
- \ref Advanced

\section Disclaimer

This is a work in progress.

\section Introduction

Here is a simple program that outputs a note-on MIDI message

\code
#include <QApplication>
#include <aseqmm.h>

int main(int argc, char **argv) {
    QApplication app(argc, argv, false);

    // initialize the client
    MidiClient *client = new MidiClient();
    client->setOpenMode(SND_SEQ_OPEN_DUPLEX);
    client->setBlockMode(false);
    client->open();
    client->setClientName("MyClient");

    // initialize the port
    MidiPort *port = client->createPort();
    port->setPortName("MyPort");
    port->setCapability(SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ);
    port->setPortType(SND_SEQ_PORT_TYPE_APPLICATION | SND_SEQ_PORT_TYPE_MIDI_GENERIC);
    port->attach();
    // subscribe the port to some other client:port
    port->subscribeTo("20:0");

    // create and send a note on message
    NoteOnEvent ev(0, 66, 100);
    ev.setSource(port->getPortId());
    ev.setSubscribers();
    ev.setDirect();
    client->output(&ev);
    client->drainOutput();

    // close and clean the created instances
    client->close();
    delete port;
    delete client;
    return 0;
}
\endcode

\section Advanced Advanced features, not yet documented

A lot. Almost all.

\example dumpmid.cpp
Print received sequencer events

\example playsmf.cpp
SMF playback, command line interface program

\example smfplayer.cpp
SMF playback, graphic user interface program

\example buildsmf.cpp
SMF output from scratch

\example dumpsmf.cpp
SMF read and print

\example sysinfo.cpp
Prints information about the ALSA sequencer subsystem

\example testevents.cpp
SequencerEvents test

\example timertest.cpp
ALSA Timers test

\example vpiano.cpp
A Virtual Piano Keyboard GUI application. See another one at http://vmpk.sf.net
*/

#include "alsaclient.h"
#include "alsaqueue.h"
#include "alsaevent.h"
#include "alsaport.h"
#include <QThread>
#include <QApplication>

/**
 * @class QObject
 * @brief The QObject class is the base class of all Qt objects.
 * @see   http://doc.trolltech.com/qobject.html
 */

/**
 * @class QThread
 * @brief The QThread class provides platform-independent threads.
 * @see   http://doc.trolltech.com/qthread.html
 */

BEGIN_ALSASEQ_NAMESPACE

/*
 * MidiClient
 */

/**
 * This constructor initialize several members with default values, but it is
 * necessary to invoke open() later to get the sequencer client handler from
 * the ALSA sequencer subsystem.
 *
 * @param parent The parent object
 * @return a MidiClient instance
 */
MidiClient::MidiClient( QObject* parent ) :
    QObject(parent),
    m_eventsEnabled(false),
    m_BlockMode(false),
    m_NeedRefreshClientList(true),
    m_OpenMode(SND_SEQ_OPEN_DUPLEX),
    m_DeviceName("default"),
    m_SeqHandle(NULL),
    m_Thread(NULL),
    m_Queue(NULL),
    m_handler(NULL)
{ }

/**
 * @brief Destructor
 *
 * The ports and queue associated to this client are automatically released.
 */
MidiClient::~MidiClient()
{
    stopSequencerInput();
    detachAllPorts();
    if (m_Queue != NULL)
        delete m_Queue;
    close();
    freeClients();
    if (m_Thread != NULL)
        delete m_Thread;
}

/**
 * @brief Open the sequencer device getting a handle
 *
 * Before opening the MidiClient instance, several properties should be set
 * as the device name (m_DeviceName), the open mode and block mode.
 */
void
MidiClient::open()
{
    CHECK_ERROR(snd_seq_open(&m_SeqHandle, m_DeviceName.toLocal8Bit().data(),
                             m_OpenMode, m_BlockMode ? 0 : SND_SEQ_NONBLOCK));
    CHECK_WARNING(snd_seq_get_client_info(m_SeqHandle, m_Info.m_Info));
}

/**
 * @brief Open the sequencer device getting a handle, providing a configuration
 * object pointer.
 *
 * This method is like open() except that the configuration can be explicitly
 * provided.
 *
 * @param conf
 */
void
MidiClient::open(snd_config_t* conf)
{
    CHECK_ERROR(snd_seq_open_lconf( &m_SeqHandle,
                                    m_DeviceName.toLocal8Bit().data(),
                                    m_OpenMode,
                                    m_BlockMode ? 0 : SND_SEQ_NONBLOCK,
                                    conf ));
    CHECK_WARNING(snd_seq_get_client_info(m_SeqHandle, m_Info.m_Info));
}

/**
 * Close the sequencer device
 */
void
MidiClient::close()
{
    if (m_SeqHandle != NULL) {
        stopSequencerInput();
        CHECK_WARNING(snd_seq_close(m_SeqHandle));
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
        CHECK_WARNING(snd_seq_set_output_buffer_size(m_SeqHandle, newSize));
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
        CHECK_WARNING(snd_seq_set_input_buffer_size(m_SeqHandle, newSize));
    }
}

void
MidiClient::setDeviceName( QString const& newName)
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
    if (m_BlockMode != newValue)
    {
        m_BlockMode = newValue;
        if (m_SeqHandle != NULL)
        {
            CHECK_WARNING(snd_seq_nonblock(m_SeqHandle, m_BlockMode ? 0 : 1));
        }
    }
}

int
MidiClient::getClientId()
{
    return CHECK_WARNING(snd_seq_client_id(m_SeqHandle));
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
            // first, process the callback (if any)
            if (m_handler != NULL)
                m_handler->handleSequencerEvent(event->clone());
            // second, process the event listeners
            if (m_eventsEnabled) {
               QObjectList::Iterator it;
                for(it=m_listeners.begin(); it!=m_listeners.end(); ++it) {
                    QObject* sub = (*it);
                    QApplication::postEvent(sub, event->clone());
                }
            }
            // finally, process signals
            emit eventReceived(event->clone());
            delete event;
        }
    }
    while (snd_seq_event_input_pending(m_SeqHandle, 0) > 0);
}

void
MidiClient::startSequencerInput()
{
    if (m_Thread == NULL) {
        m_Thread = new SequencerInputThread(this, 500);
        m_Thread->start();
    }
}

void
MidiClient::stopSequencerInput()
{
    int counter = 0;
    if (m_Thread != NULL) {
        m_Thread->stop();
        while (!m_Thread->wait(500) && (counter < 10)) {
            counter++;
        }
        if (!m_Thread->isFinished()) {
            m_Thread->terminate();
        }
        delete m_Thread;
    }
}

void
MidiClient::readClients()
{
    ClientInfo cInfo;
    freeClients();
    cInfo.setClient(-1);
    while (snd_seq_query_next_client(m_SeqHandle, cInfo.m_Info) >= 0) {
        cInfo.readPorts(this);
        m_ClientList.append(cInfo);
    }
    m_NeedRefreshClientList = false;
}

void
MidiClient::freeClients()
{
    m_ClientList.clear();
}

ClientInfoList
MidiClient::getAvailableClients()
{
    if (m_NeedRefreshClientList)
        readClients();
    ClientInfoList lst = m_ClientList; // copy
    return lst;
}

ClientInfo&
MidiClient::getThisClientInfo()
{
    snd_seq_get_client_info(m_SeqHandle, m_Info.m_Info);
    return m_Info;
}

void
MidiClient::setThisClientInfo(const ClientInfo& val)
{
    m_Info = val;
    snd_seq_set_client_info(m_SeqHandle, m_Info.m_Info);
}

void
MidiClient::applyClientInfo()
{
    if (m_SeqHandle != NULL) {
        snd_seq_set_client_info(m_SeqHandle, m_Info.m_Info);
    }
}

QString
MidiClient::getClientName()
{
    return m_Info.getName();
}

QString MidiClient::getClientName(const int clientId)
{
    ClientInfoList::Iterator it;
    if (m_NeedRefreshClientList) readClients();
    for (it = m_ClientList.begin(); it != m_ClientList.end(); ++it) {
        if ((*it).getClientId() == clientId) {
            return (*it).getName();
        }
    }
    return QString();
}

void
MidiClient::setClientName(QString const& newName)
{
    if (newName != m_Info.getName()) {
        m_Info.setName(newName);
        applyClientInfo();
    }
}

MidiPortList
MidiClient::getMidiPorts() const
{
    MidiPortList lst = m_Ports;  // copy
    return lst;
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
        CHECK_ERROR(snd_seq_create_port(m_SeqHandle, port->m_Info.m_Info));
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

        MidiPortList::iterator it;
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
        MidiPortList::iterator it;
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
    return m_Info.getBroadcastFilter();
}

void
MidiClient::setBroadcastFilter(bool newValue)
{
    m_Info.setBroadcastFilter(newValue);
    applyClientInfo();
}

bool
MidiClient::getErrorBounce()
{
    return m_Info.getErrorBounce();
}

void
MidiClient::setErrorBounce(bool newValue)
{
    m_Info.setErrorBounce(newValue);
    applyClientInfo();
}

void MidiClient::output(SequencerEvent* ev, bool async, int timeout)
{
    int npfds;
    pollfd* pfds;
    if (async) {
        CHECK_WARNING(snd_seq_event_output(m_SeqHandle, ev->getHandle()));
    } else {
        npfds = snd_seq_poll_descriptors_count(m_SeqHandle, POLLOUT);
        pfds = (pollfd*) alloca(npfds * sizeof(pollfd));
        snd_seq_poll_descriptors(m_SeqHandle, pfds, npfds, POLLOUT);
        while (snd_seq_event_output(m_SeqHandle, ev->getHandle()) < 0)
        {
            poll(pfds, npfds, timeout);
        }
    }
}

void MidiClient::outputDirect(SequencerEvent* ev, bool async, int timeout)
{
    int npfds;
    pollfd* pfds;
    if (async) {
        CHECK_WARNING(snd_seq_event_output_direct(m_SeqHandle, ev->getHandle()));
    } else {
        npfds = snd_seq_poll_descriptors_count(m_SeqHandle, POLLOUT);
        pfds = (pollfd*) alloca(npfds * sizeof(pollfd));
        snd_seq_poll_descriptors(m_SeqHandle, pfds, npfds, POLLOUT);
        while (snd_seq_event_output_direct(m_SeqHandle, ev->getHandle()) < 0)
        {
            poll(pfds, npfds, timeout);
        }
    }
}

void MidiClient::outputBuffer(SequencerEvent* ev, bool async, int timeout)
{
    int npfds;
    pollfd* pfds;
    if (async) {
        CHECK_WARNING(snd_seq_event_output_buffer(m_SeqHandle, ev->getHandle()));
    } else {
        npfds = snd_seq_poll_descriptors_count(m_SeqHandle, POLLOUT);
        pfds = (pollfd*) alloca(npfds * sizeof(pollfd));
        snd_seq_poll_descriptors(m_SeqHandle, pfds, npfds, POLLOUT);
        while (snd_seq_event_output_buffer(m_SeqHandle, ev->getHandle()) < 0)
        {
            poll(pfds, npfds, timeout);
        }
    }
}

void MidiClient::drainOutput(bool async, int timeout)
{
    int npfds;
    pollfd* pfds;
    if (async) {
        CHECK_WARNING(snd_seq_drain_output(m_SeqHandle));
    } else {
        npfds = snd_seq_poll_descriptors_count(m_SeqHandle, POLLOUT);
        pfds = (pollfd*) alloca(npfds * sizeof(pollfd));
        snd_seq_poll_descriptors(m_SeqHandle, pfds, npfds, POLLOUT);
        while (snd_seq_drain_output(m_SeqHandle) < 0)
        {
            poll(pfds, npfds, timeout);
        }
    }
}

void
MidiClient::synchronizeOutput()
{
    snd_seq_sync_output_queue(m_SeqHandle);
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
    m_Queue = new MidiQueue(this, this);
    return m_Queue;
}

MidiQueue*
MidiClient::createQueue(QString const& queueName )
{
    if (m_Queue != NULL) {
        delete m_Queue;
    }
    m_Queue = new MidiQueue(this, queueName, this);
    return m_Queue;
}

MidiQueue*
MidiClient::useQueue(int queue_id)
{
    if (m_Queue != NULL) {
        delete m_Queue;
    }
    m_Queue = new MidiQueue(this, queue_id, this);
    return m_Queue;
}

MidiQueue*
MidiClient::useQueue(const QString& name)
{
    if (m_Queue != NULL) {
        delete m_Queue;
    }
    int queue_id = getQueueId(name);
    if ( queue_id >= 0) {
       m_Queue = new MidiQueue(this, queue_id, this);
    }
    return m_Queue;
}

MidiQueue*
MidiClient::useQueue(MidiQueue* queue)
{
    if (m_Queue != NULL) {
        delete m_Queue;
    }
    queue->setParent(this);
    m_Queue = queue;
    return m_Queue;
}

QList<int> MidiClient::getAvailableQueues()
{
    int q, err, max;
    QList<int> queues;
    snd_seq_queue_info_t* qinfo;
    snd_seq_queue_info_alloca(&qinfo);
    max = getSystemInfo().getMaxQueues();
    for ( q = 0; q < max; ++q ) {
        err = snd_seq_get_queue_info(m_SeqHandle, q, qinfo);
        if (err == 0) {
            queues.append(q);
        }
    }
    return queues;
}

PortInfoList
MidiClient::filterPorts(unsigned int filter)
{
    PortInfoList result;
    ClientInfoList::ConstIterator itc;
    PortInfoList::ConstIterator itp;

    if (m_NeedRefreshClientList)
        readClients();

    for (itc = m_ClientList.begin(); itc != m_ClientList.end(); ++itc) {
        ClientInfo ci = (*itc);
        if ((ci.getClientId() == SND_SEQ_CLIENT_SYSTEM) ||
            (ci.getClientId() == m_Info.getClientId()))
            continue;
        PortInfoList lstPorts = ci.getPorts();
        for(itp = lstPorts.begin(); itp != lstPorts.end(); ++itp) {
            PortInfo pi = (*itp);
            unsigned int cap = pi.getCapability();
            if ( ((filter & cap) != 0) &&
                 ((SND_SEQ_PORT_CAP_NO_EXPORT & cap) == 0) ) {
                result.append(pi);
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
    m_InputsAvail = filterPorts( SND_SEQ_PORT_CAP_READ |
                                 SND_SEQ_PORT_CAP_SUBS_READ );
    m_OutputsAvail = filterPorts( SND_SEQ_PORT_CAP_WRITE |
                                  SND_SEQ_PORT_CAP_SUBS_WRITE );
}

PortInfoList
MidiClient::getAvailableInputs()
{
    if (m_NeedRefreshClientList || m_InputsAvail.empty()) {
        updateAvailablePorts();
    }
    return m_InputsAvail;
}

PortInfoList
MidiClient::getAvailableOutputs()
{
    if (m_NeedRefreshClientList || m_OutputsAvail.empty()) {
        updateAvailablePorts();
    }
    return m_OutputsAvail;
}

void
MidiClient::addListener(QObject* listener)
{
    m_listeners.append(listener);
}

void
MidiClient::removeListener(QObject* listener)
{
    m_listeners.removeAll(listener);
}

void
MidiClient::setEventsEnabled(bool bEnabled)
{
    if (bEnabled != m_eventsEnabled) {
        m_eventsEnabled = (bEnabled & !m_listeners.empty());
    }
}

SystemInfo&
MidiClient::getSystemInfo()
{
    snd_seq_system_info(m_SeqHandle, m_sysInfo.m_Info);
    return m_sysInfo;
}

PoolInfo&
MidiClient::getPoolInfo()
{
    snd_seq_get_client_pool(m_SeqHandle, m_poolInfo.m_Info);
    return m_poolInfo;
}

void
MidiClient::setPoolInfo(const PoolInfo& info)
{
    m_poolInfo = info;
    CHECK_WARNING(snd_seq_set_client_pool(m_SeqHandle, m_poolInfo.m_Info));
}

void
MidiClient::resetPoolInput()
{
    CHECK_WARNING(snd_seq_reset_pool_input(m_SeqHandle));
}

void
MidiClient::resetPoolOutput()
{
    CHECK_WARNING(snd_seq_reset_pool_output(m_SeqHandle));
}

void
MidiClient::setPoolInput(int size)
{
    CHECK_WARNING(snd_seq_set_client_pool_input(m_SeqHandle, size));
}

void
MidiClient::setPoolOutput(int size)
{
    CHECK_WARNING(snd_seq_set_client_pool_output(m_SeqHandle, size));
}

void
MidiClient::setPoolOutputRoom(int size)
{
    CHECK_WARNING(snd_seq_set_client_pool_output_room(m_SeqHandle, size));
}

void
MidiClient::dropInput()
{
    CHECK_WARNING(snd_seq_drop_input(m_SeqHandle));
}

void
MidiClient::dropInputBuffer()
{
    CHECK_WARNING(snd_seq_drop_input_buffer(m_SeqHandle));
}

void
MidiClient::dropOutput()
{
    CHECK_WARNING(snd_seq_drop_output(m_SeqHandle));
}

void
MidiClient::dropOutputBuffer()
{
    CHECK_WARNING(snd_seq_drop_output_buffer(m_SeqHandle));
}

void
MidiClient::removeEvents(const RemoveEvents* spec)
{
    CHECK_WARNING(snd_seq_remove_events(m_SeqHandle, spec->m_Info));
}

SequencerEvent*
MidiClient::extractOutput()
{
    snd_seq_event_t* ev;
    if (CHECK_WARNING(snd_seq_extract_output(m_SeqHandle, &ev) == 0)) {
        return new SequencerEvent(ev);
    }
    return NULL;
}

int
MidiClient::outputPending()
{
    return snd_seq_event_output_pending(m_SeqHandle);
}

int
MidiClient::inputPending(bool fetch)
{
    return snd_seq_event_input_pending(m_SeqHandle, fetch ? 1 : 0);
}

int
MidiClient::getQueueId(const QString& name)
{
    return snd_seq_query_named_queue(m_SeqHandle, name.toLocal8Bit().data());
}

int
MidiClient::getPollDescriptorsCount(short events)
{
    return snd_seq_poll_descriptors_count(m_SeqHandle, events);
}

int
MidiClient::pollDescriptors( struct pollfd *pfds, unsigned int space,
                             short events )
{
    return snd_seq_poll_descriptors(m_SeqHandle, pfds, space, events);
}

unsigned short
MidiClient::pollDescriptorsRevents(struct pollfd *pfds, unsigned int nfds)
{
    unsigned short revents;
    CHECK_WARNING( snd_seq_poll_descriptors_revents( m_SeqHandle,
                                                     pfds, nfds,
                                                     &revents ));
    return revents;
}

const char *
MidiClient::_getDeviceName()
{
    return snd_seq_name(m_SeqHandle);
}

void
MidiClient::_setClientName(const char *name)
{
    CHECK_WARNING(snd_seq_set_client_name(m_SeqHandle, name));
}

int
MidiClient::createSimplePort( const char *name,
                              unsigned int caps,
                              unsigned int type )
{
    return CHECK_WARNING( snd_seq_create_simple_port( m_SeqHandle,
                                                      name, caps, type ));
}

void
MidiClient::deleteSimplePort(int port)
{
    CHECK_WARNING( snd_seq_delete_simple_port( m_SeqHandle, port ));
}

void
MidiClient::connectFrom(int myport, int client, int port)
{
    CHECK_WARNING( snd_seq_connect_from(m_SeqHandle, myport, client, port ));
}

void
MidiClient::connectTo(int myport, int client, int port)
{
    CHECK_WARNING( snd_seq_connect_to(m_SeqHandle, myport, client, port ));
}

void
MidiClient::disconnectFrom(int myport, int client, int port)
{
    CHECK_WARNING( snd_seq_disconnect_from(m_SeqHandle, myport, client, port ));
}

void
MidiClient::disconnectTo(int myport, int client, int port)
{
    CHECK_WARNING( snd_seq_disconnect_to(m_SeqHandle, myport, client, port ));
}

/* ******************** *
 * SequencerInputThread *
 * ******************** */

bool MidiClient::SequencerInputThread::stopped()
{
    m_mutex.lockForRead();
    bool bTmp = m_Stopped;
    m_mutex.unlock();
    return  bTmp;
}

void MidiClient::SequencerInputThread::stop()
{
    m_mutex.lockForWrite();
    m_Stopped = true;
    m_mutex.unlock();
}

void MidiClient::SequencerInputThread::run()
{
    unsigned long npfd;
    pollfd* pfd;
    int rt;

    if (m_MidiClient != NULL) {
        npfd = snd_seq_poll_descriptors_count(m_MidiClient->getHandle(), POLLIN);
        pfd = (pollfd *) alloca(npfd * sizeof(pollfd));
        try
        {
            snd_seq_poll_descriptors(m_MidiClient->getHandle(), pfd, npfd, POLLIN);
            while (!stopped() && (m_MidiClient != NULL))
            {
                rt = poll(pfd, npfd, m_Wait);
                if (rt > 0) {
                    m_MidiClient->doEvents();
                }
            }
        }
        catch (...)
        {
            qWarning() << "exception in input thread";
        }
    }
}

/**************/
/* ClientInfo */
/**************/

ClientInfo::ClientInfo()
{
    snd_seq_client_info_malloc(&m_Info);
}

ClientInfo::ClientInfo(const ClientInfo& other)
{
    snd_seq_client_info_malloc(&m_Info);
    snd_seq_client_info_copy(m_Info, other.m_Info);
    m_Ports = other.m_Ports;
}

ClientInfo::ClientInfo(snd_seq_client_info_t* other)
{
    snd_seq_client_info_malloc(&m_Info);
    snd_seq_client_info_copy(m_Info, other);
}

ClientInfo::ClientInfo(MidiClient* seq, int id)
{
    snd_seq_client_info_malloc(&m_Info);
    snd_seq_get_any_client_info(seq->getHandle(), id, m_Info);
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

ClientInfo&
ClientInfo::operator=(const ClientInfo& other)
{
    snd_seq_client_info_copy(m_Info, other.m_Info);
    m_Ports = other.m_Ports;
    return *this;
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

QString
ClientInfo::getName()
{
    return QString(snd_seq_client_info_get_name(m_Info));
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
ClientInfo::setName(QString name)
{
    snd_seq_client_info_set_name(m_Info, name.toLocal8Bit().data());
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
    PortInfo info;
    freePorts();
    info.setClient(getClientId());
    info.setClientName(getName());
    info.setPort(-1);
    while (snd_seq_query_next_port(seq->getHandle(), info.m_Info) >= 0) {
        info.readSubscribers(seq);
        m_Ports.append(info);
    }
}

void
ClientInfo::freePorts()
{
    m_Ports.clear();
}

PortInfoList
ClientInfo::getPorts() const
{
    PortInfoList lst = m_Ports; // copy
    return lst;
}

int
ClientInfo::getSizeOfInfo() const
{
    return snd_seq_client_info_sizeof();
}

#if SND_LIB_SUBMINOR > 16
void
ClientInfo::addFilter(int eventType)
{
    snd_seq_client_info_event_filter_add(m_Info, eventType);
}

bool
ClientInfo::isFiltered(int eventType)
{
    return (snd_seq_client_info_event_filter_check(m_Info, eventType) != 0);
}

void
ClientInfo::clearFilter()
{
    snd_seq_client_info_event_filter_clear(m_Info);
}

void
ClientInfo::removeFilter(int eventType)
{
    snd_seq_client_info_event_filter_del(m_Info, eventType);
}
#endif

/**************
 * SystemInfo *
 **************/

SystemInfo::SystemInfo()
{
    snd_seq_system_info_malloc(&m_Info);
}

SystemInfo::SystemInfo(const SystemInfo& other)
{
    snd_seq_system_info_malloc(&m_Info);
    snd_seq_system_info_copy(m_Info, other.m_Info);
}

SystemInfo::SystemInfo(snd_seq_system_info_t* other)
{
    snd_seq_system_info_malloc(&m_Info);
    snd_seq_system_info_copy(m_Info, other);
}

SystemInfo::SystemInfo(MidiClient* seq)
{
    snd_seq_system_info_malloc(&m_Info);
    snd_seq_system_info(seq->getHandle(), m_Info);
}

SystemInfo::~SystemInfo()
{
    snd_seq_system_info_free(m_Info);
}

SystemInfo*
SystemInfo::clone()
{
    return new SystemInfo(m_Info);
}

SystemInfo&
SystemInfo::operator=(const SystemInfo& other)
{
    snd_seq_system_info_copy(m_Info, other.m_Info);
    return *this;
}

int SystemInfo::getMaxClients()
{
    return snd_seq_system_info_get_clients(m_Info);
}

int SystemInfo::getMaxPorts()
{
    return snd_seq_system_info_get_ports(m_Info);
}

int SystemInfo::getMaxQueues()
{
    return snd_seq_system_info_get_queues(m_Info);
}

int SystemInfo::getMaxChannels()
{
    return snd_seq_system_info_get_channels(m_Info);
}

int SystemInfo::getCurrentQueues()
{
    return snd_seq_system_info_get_cur_queues(m_Info);
}

int SystemInfo::getCurrentClients()
{
    return snd_seq_system_info_get_cur_clients(m_Info);
}

int SystemInfo::getSizeOfInfo() const
{
    return snd_seq_system_info_sizeof();
}

/************
 * PoolInfo *
 ************/

PoolInfo::PoolInfo()
{
    snd_seq_client_pool_malloc(&m_Info);
}

PoolInfo::PoolInfo(const PoolInfo& other)
{
    snd_seq_client_pool_malloc(&m_Info);
    snd_seq_client_pool_copy(m_Info, other.m_Info);
}

PoolInfo::PoolInfo(snd_seq_client_pool_t* other)
{
    snd_seq_client_pool_malloc(&m_Info);
    snd_seq_client_pool_copy(m_Info, other);
}

PoolInfo::PoolInfo(MidiClient* seq)
{
    snd_seq_client_pool_malloc(&m_Info);
    snd_seq_get_client_pool(seq->getHandle(), m_Info);
}

PoolInfo::~PoolInfo()
{
    snd_seq_client_pool_free(m_Info);
}

PoolInfo* PoolInfo::clone()
{
    return new PoolInfo(m_Info);
}

PoolInfo& PoolInfo::operator=(const PoolInfo& other)
{
    snd_seq_client_pool_copy(m_Info, other.m_Info);
    return *this;
}

int PoolInfo::getClientId()
{
    return snd_seq_client_pool_get_client(m_Info);
}

int PoolInfo::getInputFree()
{
    return snd_seq_client_pool_get_input_free(m_Info);
}

int PoolInfo::getInputPool()
{
    return snd_seq_client_pool_get_input_pool(m_Info);
}

int PoolInfo::getOutputFree()
{
    return snd_seq_client_pool_get_output_free(m_Info);
}

int PoolInfo::getOutputPool()
{
    return snd_seq_client_pool_get_output_pool(m_Info);
}

int PoolInfo::getOutputRoom()
{
    return snd_seq_client_pool_get_output_room(m_Info);
}

void PoolInfo::setInputPool(int size)
{
    snd_seq_client_pool_set_input_pool(m_Info, size);
}

void PoolInfo::setOutputPool(int size)
{
    snd_seq_client_pool_set_output_pool(m_Info, size);
}

void PoolInfo::setOutputRoom(int size)
{
    snd_seq_client_pool_set_output_room(m_Info, size);
}

int PoolInfo::getSizeOfInfo() const
{
    return snd_seq_client_pool_sizeof();
}

END_ALSASEQ_NAMESPACE

