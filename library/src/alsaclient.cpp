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

#include "alsaclient.h"
#include "alsaqueue.h"
#include "alsaevent.h"
#include "alsaport.h"
#include <QThread>
#include <QApplication>

namespace aseqmm {

/**
@mainpage aseqmm Documentation
@author Copyright &copy; 2009 Pedro López-Cabanillas &lt;plcl AT users.sf.net&gt;
@version 0.0.3
@date 2009-08-12

This document is licensed under the Creative Commons Attribution-Share Alike 3.0 Unported License.
To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/

@section Abstract

This is the reference documentation for aseqmm. This library is a C++ wrapper
around the ALSA library sequencer interface, using Qt4 objects, idioms and style.

@see http://doc.trolltech.com/index.html
@see http://www.alsa-project.org/alsa-doc/alsa-lib/seq.html
@see http://cartan.cas.suffolk.edu/oopdocbook/opensource/index.html

@section Disclaimer

This document is a work in progress, in a very early state. It will be always in
development. Please visit the aseqmm web site to read the latest version.

@see http://kmetronome.sourceforge.net/aseqmm/

@section Introduction

For an introduction to design and programming with C++ and Qt4, see the book
"An Introduction to Design Patterns in C++ with Qt 4" by by Alan Ezust and Paul
Ezust. It is available published on dead trees, and also
<a href="http://cartan.cas.suffolk.edu/oopdocbook/opensource/index.html">
online</a>.

Here is how a simple program playing a note-on MIDI message using aseqmm
looks like:

@code
#include <QApplication>
#include <aseqmm.h>

int main(int argc, char **argv) {
    QApplication app(argc, argv, false);

    // create a client object on the heap
    aseqmm::MidiClient *client = new aseqmm::MidiClient();
    client->open();
    client->setClientName( "MyClient" );

    // create the port
    aseqmm::MidiPort *port = client->createPort();
    port->setPortName( "MyPort" );
    port->setCapability( SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ );
    port->setPortType( SND_SEQ_PORT_TYPE_APPLICATION | SND_SEQ_PORT_TYPE_MIDI_GENERIC );
    // subscribe the port to some other client:port
    port->subscribeTo( "20:0" ); // or "name:port", like in "KMidimon:0"

    // create an event object on the stack, to send a note on message
    aseqmm::NoteOnEvent ev( 0, 66, 100 ); // (channel, note number, velocity)
    ev.setSource( port->getPortId() );
    ev.setSubscribers();
    ev.setDirect(); // not scheduled
    client->output( &ev ); // or outputDirect() if you prefer not buffered
    client->drainOutput();

    // close and clean the created instances
    client->close();
    delete port;
    delete client;
    return 0;
}
@endcode

There are more examples in the source tree, under the tests/ directory, and
you can also see applications using this library, as kmetronome and kmidimon.

@see http://kmetronome.sourceforge.net/
@see http://kmetronome.sourceforge.net/kmidimon/

@section Advanced Advanced features, not yet documented

A lot. Almost all.

@example dumpmid.cpp
Print received sequencer events

@example playsmf.cpp
SMF playback, command line interface program

@example smfplayer.cpp
SMF playback, graphic user interface program

@example buildsmf.cpp
SMF output from scratch

@example dumpsmf.cpp
SMF read and print

@example sysinfo.cpp
Prints information about the ALSA sequencer subsystem

@example testevents.cpp
SequencerEvents test

@example timertest.cpp
ALSA Timers test

@example vpiano.cpp
A Virtual Piano Keyboard GUI application. See another one at http://vmpk.sf.net
*/

/**
 * @class QObject
 * The QObject class is the base class of all Qt objects.
 * @see   http://doc.trolltech.com/qobject.html
 */

/**
 * @class QThread
 * The QThread class provides platform-independent threads.
 * @see   http://doc.trolltech.com/qthread.html
 */

/**
 * @addtogroup ALSAClient
 *
 * ALSA clients are any entities using ALSA sequencer services. A client
 * may be an application or a device driver for an external MIDI port, like
 * USB MIDI devices or the MIDI/game ports of some sound cards. This library
 * allows to easily create applications managing ALSA clients.
 *
 * ALSA clients are also a file descriptor representing a sequencer device,
 * that must be opened before reading or writing MIDI events. When the client
 * is opened, it is given some handle and a number identifying it to other
 * clients in the system. You can also provide a name for it.
 *
 * The ALSA sequencer clients can have some ports attached. The ports can be
 * readable or writable, and can be subscribed in pairs: one readable port to
 * one writable port. The subscriptions can be made and queried by external
 * applications, like "aconnect" or "qjackctl".
 *
 * SystemInfo is an auxiliary class to query several system capabilities.
 *
 * The PoolInfo class represents a container to query and change some values
 * for the kernel memory pool assigned to an ALSA client.
 *
 * The ClientInfo class is another container to query and change values about
 * the MidiClient itself.
 *
 * The SequencerEventHandler abstract class is used to define an interface
 * that other class can implement to receive sequencer events. It is one of the
 * three methods of delivering events offered by the library.
 *
 * @section EventInput Sequencer Events Input
 * There are three methods of events delivering:
 * <ul>
 * <li>A Callback method. To use this method, you must derive a class from
 * SequencerEventHandler, overriding the method
 * SequencerEventHandler::handleSequencerEvent() to
 * provide your own event processing. You must provide the handler instance to
 * the client using setHandler().</li>
 * <li>Using QEvent listeners. To use this method, you must use one or more
 * classes derived from QObject overriding the method QObject::customEvent().
 * You must also use the method addListener() to add such objects to the
 * client's listeners list.</li>
 * <li>The third method involves signals and slots. Whenever a sequencer event
 * is received, a signal eventReceived() is emitted, that can be connected to
 * your own supplied slot(s) to process it.
 * </ul>
 *
 */

/**
 * Constructor.
 *
 * This constructor optionally gets a QObject parent. When you create a
 * MidiClient with another object as parent, the MidiClient object will
 * automatically add itself to the parent's children() list. The parent takes
 * ownership of the object i.e. it will automatically delete its children in
 * its destructor.
 *
 * It is necessary to invoke open() later to get the sequencer client handler
 * from the ALSA sequencer subsystem.
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
 * Destructor.
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
 * Open the sequencer device.
 *
 * When opening the MidiClient instance, several properties may optionally
 * be set as the device name, the open mode and block mode. Default values
 * are provided for them. After a successful open, an event with
 * SND_SEQ_EVENT_CLIENT_START is broadcast to the announce port.
 *
 * @param deviceName the sequencer device name, default value = &quot;default&quot;.
 * This is not a name you make up for your own purposes; it has special
 * significance to the ALSA library. Usually you need to pass &quot;default&quot; here.
 * @param openMode the open mode, default value = SND_SEQ_OPEN_DUPLEX.
 * The read/write mode of the sequencer. Can be one of these three values:
 * <ul>
 * <li>SND_SEQ_OPEN_OUTPUT - open the sequencer for output only</li>
 * <li>SND_SEQ_OPEN_INPUT - open the sequencer for input only</li>
 * <li>SND_SEQ_OPEN_DUPLEX - open the sequencer for output and input</li>
 * </ul>
 * @param blockMode open in blocking mode, default value = false.
 */
void
MidiClient::open( const QString deviceName,
                  const int openMode,
                  const bool blockMode)
{
    CHECK_ERROR( snd_seq_open( &m_SeqHandle, deviceName.toLocal8Bit().data(),
                              openMode, blockMode ? 0 : SND_SEQ_NONBLOCK ) );
    CHECK_WARNING( snd_seq_get_client_info( m_SeqHandle, m_Info.m_Info ) );
    m_DeviceName = deviceName;
    m_OpenMode = openMode;
    m_BlockMode = blockMode;
}

/**
 * Open the sequencer device, providing a configuration object pointer.
 *
 * This method is like open() except that a configuration can be explicitly
 * provided. After a successful open, an event with SND_SEQ_EVENT_CLIENT_START
 * is broadcast to the announce port.
 *
 * @param conf a configuration object pointer.
 * @param deviceName the sequencer device name, default value = &quot;default&quot;.
 * This is not a name you make up for your own purposes; it has special
 * significance to the ALSA library. Usually you need to pass &quot;default&quot; here.
 * @param openMode the open mode, default value = SND_SEQ_OPEN_DUPLEX.
 * The read/write mode of the sequencer. Can be one of these three values:
 * <ul>
 * <li>SND_SEQ_OPEN_OUTPUT - open the sequencer for output only</li>
 * <li>SND_SEQ_OPEN_INPUT - open the sequencer for input only</li>
 * <li>SND_SEQ_OPEN_DUPLEX - open the sequencer for output and input</li>
 * </ul>
 * @param blockMode open in blocking mode, default value = false.
 */
void
MidiClient::open( snd_config_t* conf,
                  const QString deviceName,
                  const int openMode,
                  const bool blockMode )
{
    CHECK_ERROR( snd_seq_open_lconf( &m_SeqHandle,
                                     deviceName.toLocal8Bit().data(),
                                     openMode,
                                     blockMode ? 0 : SND_SEQ_NONBLOCK,
                                     conf ));
    CHECK_WARNING( snd_seq_get_client_info(m_SeqHandle, m_Info.m_Info));
    m_DeviceName = deviceName;
    m_OpenMode = openMode;
    m_BlockMode = blockMode;
}

/**
 * Close the sequencer device.
 *
 * After a client is closed, an event with SND_SEQ_EVENT_CLIENT_EXIT is
 * broadcast to the announce port. The connection between other clients are
 * disconnected. Call this just before exiting your program.
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

/**
 * Gets the size of the library output buffer for the ALSA client.
 *
 * This buffer is used to store the decoded byte-stream of output events before
 * transferring to the sequencer.
 *
 * @return the size of the library output buffer
 */
size_t
MidiClient::getOutputBufferSize()
{
    return snd_seq_get_output_buffer_size(m_SeqHandle);
}

/**
 * Sets the size of the library output buffer for the ALSA client.
 *
 * This buffer is used to store the decoded byte-stream of output events before
 * transferring to the sequencer.
 *
 * @param newSize the size of the library output buffer
 */
void
MidiClient::setOutputBufferSize(size_t newSize)
{
    if (getOutputBufferSize() != newSize) {
        CHECK_WARNING(snd_seq_set_output_buffer_size(m_SeqHandle, newSize));
    }
}

/**
 * Gets the size of the library input buffer for the ALSA client.
 *
 * This buffer is used to read a byte-stream of input events before
 * transferring from the sequencer.
 *
 * @return the size of the library input buffer
 */
size_t
MidiClient::getInputBufferSize()
{
    return snd_seq_get_input_buffer_size(m_SeqHandle);
}

/**
 * Sets the size of the library input buffer for the ALSA client.
 *
 * This buffer is used to read a byte-stream of input events before
 * transferring from the sequencer.
 *
 * @param newSize the size of the library input buffer
 */
void
MidiClient::setInputBufferSize(size_t newSize)
{
    if (getInputBufferSize() != newSize) {
        CHECK_WARNING(snd_seq_set_input_buffer_size(m_SeqHandle, newSize));
    }
}

/**
 * Change the blocking mode of the client.
 *
 * In block mode, the client falls into sleep when it fills the output memory
 * pool with full events. The client will be woken up after a certain amount
 * of free space becomes available.
 *
 * @param newValue the blocking mode
 */
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

/**
 * Gets the client ID.
 *
 * Returns the ID of the client. A client ID is necessary to inquiry or to set
 * the client information. A user client ID is assigned from 128 to 191.
 *
 * @return the client ID.
 */
int
MidiClient::getClientId()
{
    return CHECK_WARNING(snd_seq_client_id(m_SeqHandle));
}

/**
 * Returns the type snd_seq_type_t of the given sequencer handle.
 * @return the type snd_seq_type_t of the given sequencer handle.
 */
snd_seq_type_t
MidiClient::getSequencerType()
{
    return snd_seq_type(m_SeqHandle);
}

/**
 * Dispatch the events received from the Sequencer.
 *
 * There are three methods of events delivering:
 * <ul>
 * <li>A Callback method. To use this method, you must derive a class from
 * SequencerEventHandler, overriding the method
 * SequencerEventHandler::handleSequencerEvent() to
 * provide your own event processing. You must provide the handler instance to
 * the client using setHandler().</li>
 * <li>Using QEvent listeners. To use this method, you must use one or more
 * classes derived from QObject overriding the method QObject::customEvent().
 * You must also use the method addListener() to add such objects to the
 * client's listeners list.</li>
 * <li>The third method involves signals and slots. Whenever a sequencer event
 * is received, a signal eventReceived() is emitted, that can be connected to
 * your own supplied slot(s) to process it.
 * </ul>
 * @see ALSAClient
 */
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

/**
 * Starts reading events from the ALSA sequencer.
 */
void
MidiClient::startSequencerInput()
{
    if (m_Thread == NULL) {
        m_Thread = new SequencerInputThread(this, 500);
        m_Thread->start();
    }
}

/**
 * Stops reading events from the ALSA sequencer.
 */
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

/**
 * Reads the ALSA sequencer's clients list.
 */
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

/**
 * Releases the list of ALSA sequencer's clients.
 */
void
MidiClient::freeClients()
{
    m_ClientList.clear();
}

/**
 * Gets the list of clients from the ALSA sequencer.
 * @return the list of clients.
 */
ClientInfoList
MidiClient::getAvailableClients()
{
    if (m_NeedRefreshClientList)
        readClients();
    ClientInfoList lst = m_ClientList; // copy
    return lst;
}

/**
 * Gets the ClientInfo object holding data about this client.
 * @return the ClientInfo object representing this client.
 */
ClientInfo&
MidiClient::getThisClientInfo()
{
    snd_seq_get_client_info(m_SeqHandle, m_Info.m_Info);
    return m_Info;
}

/**
 * Sets the data supplied by the ClientInfo object into the ALSA sequencer
 * client. This allows to change the name, capabilities, type and other data
 * in a single step.
 *
 * @param val a ClientInfo object reference
 */
void
MidiClient::setThisClientInfo(const ClientInfo& val)
{
    m_Info = val;
    snd_seq_set_client_info(m_SeqHandle, m_Info.m_Info);
}

/**
 * This internal method applies the ClientInfo data to the ALSA sequencer client
 */
void
MidiClient::applyClientInfo()
{
    if (m_SeqHandle != NULL) {
        snd_seq_set_client_info(m_SeqHandle, m_Info.m_Info);
    }
}

/**
 * Gets the client's public name
 * @return The client's name
 */
QString
MidiClient::getClientName()
{
    return m_Info.getName();
}

/**
 * Gets the public name corresponding to the given Client ID.
 * @param clientId The ID of any existing sequencer client
 * @return The client's name
 */
QString
MidiClient::getClientName(const int clientId)
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

/**
 * Changes the public name of the ALSA sequencer client.
 * @param newName A new public name
 */
void
MidiClient::setClientName(QString const& newName)
{
    if (newName != m_Info.getName()) {
        m_Info.setName(newName);
        applyClientInfo();
    }
}

/**
 * Gets the list of MidiPort instances belonging to this client.
 * @return The list of MidiPort instances.
 */
MidiPortList
MidiClient::getMidiPorts() const
{
    return m_Ports;
}

/**
 * Create and attach a new MidiPort instance to this client.
 * @return The pointer to the new MidiPort instance.
 */
MidiPort*
MidiClient::createPort()
{
    MidiPort* port = new MidiPort(this);
    port->attach(this);
    return port;
}

/**
 * Attach a MidiPort instance to this client
 * @param port The MidiPort to be attached
 */
void
MidiClient::portAttach(MidiPort* port)
{
    if (m_SeqHandle != NULL) {
        CHECK_ERROR(snd_seq_create_port(m_SeqHandle, port->m_Info.m_Info));
        m_Ports.push_back(port);
    }
}

/**
 * Detach a MidiPort instance from this client
 * @param port The MidiPort to be detached
 */
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

/**
 * Detach all the ports belonging to this client.
 */
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

/**
 * Add an event filter to the client.
 * @param evtype An event filter to be added.
 */
void
MidiClient::addEventFilter(int evtype)
{
    snd_seq_set_client_event_filter(m_SeqHandle, evtype);
}

/**
 * Gets the broadcast filter usage of the client.
 *
 * @return The broadcast filter.
 */
bool
MidiClient::getBroadcastFilter()
{
    return m_Info.getBroadcastFilter();
}

/**
 * Sets the broadcast filter usage of the client.
 *
 * @param newValue The broadcast filter.
 */
void
MidiClient::setBroadcastFilter(bool newValue)
{
    m_Info.setBroadcastFilter(newValue);
    applyClientInfo();
}

/**
 * Get the error-bounce usage of the client.
 *
 * @return The error-bounce usage.
 */
bool
MidiClient::getErrorBounce()
{
    return m_Info.getErrorBounce();
}

/**
 * Sets the error-bounce usage of the client.
 *
 * @param newValue The error-bounce usage.
 */
void
MidiClient::setErrorBounce(bool newValue)
{
    m_Info.setErrorBounce(newValue);
    applyClientInfo();
}

/**
 * Output an event using the library output buffer.
 *
 * An event is once expanded on the output buffer. The output buffer will be
 * drained automatically if it becomes full.
 *
 * @param ev The event to be sent.
 * @param async Use asynchronous mode. If false, this call will block until the
 * buffer is flushed.
 * @param timeout The maximum time to wait in synchronous mode.
 */
void
MidiClient::output(SequencerEvent* ev, bool async, int timeout)
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

/**
 * Output an event directly to the sequencer
 *
 * This function sends an event to the sequencer directly not using the library
 * output buffer.
 *
 * @param ev The event to be sent.
 * @param async Use asynchronous mode. If false, this call will block until the
 * buffer is flushed.
 * @param timeout The maximum time to wait in synchronous mode.
 */
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

/**
 * Output an event using the library output buffer, without draining the buffer.
 *
 * An event is once expanded on the output buffer. The output buffer will NOT be
 * drained automatically if it becomes full.
 *
 * @param ev The event to be sent.
 * @param async Use asynchronous mode. If false, this call will block until the
 * buffer is flushed.
 * @param timeout The maximum time to wait in synchronous mode.
 */
void
MidiClient::outputBuffer(SequencerEvent* ev, bool async, int timeout)
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

/**
 * Drain the library output buffer.
 *
 * This function drains all pending events on the output buffer. The function
 * returns immediately after the events are sent to the queues regardless
 * whether the events are processed or not.
 *
 * @param async Use asynchronous mode. If false, this call will block until the
 * buffer is flushed.
 * @param timeout The maximum time to wait in synchronous mode.
 */
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

/**
 * Wait until all sent events are processed.
 *
 * This function waits until all events of this client are processed.
 */
void
MidiClient::synchronizeOutput()
{
    snd_seq_sync_output_queue(m_SeqHandle);
}

/**
 * Get the MidiQueue instance associated to this client.
 * If the client is not associated to a MidiQueue, one is created.
 * @return A MidiQueue instance pointer
 */
MidiQueue*
MidiClient::getQueue()
{
    if (m_Queue == NULL) {
        createQueue();
    }
    return m_Queue;
}

/**
 * Create and return a new MidiQueue associated to this client.
 * @return A new MidiQueue instance.
 */
MidiQueue*
MidiClient::createQueue()
{
    if (m_Queue != NULL) {
        delete m_Queue;
    }
    m_Queue = new MidiQueue(this, this);
    return m_Queue;
}

/**
 * Create and return a new MidiQueue with the given name, associated to this
 * client.
 * @param queueName The name for the new queue.
 * @return A new MidiQueue instance.
 */
MidiQueue*
MidiClient::createQueue(QString const& queueName )
{
    if (m_Queue != NULL) {
        delete m_Queue;
    }
    m_Queue = new MidiQueue(this, queueName, this);
    return m_Queue;
}

/**
 * Create a new MidiQueue instance using a queue already existing in the
 * system, associating it to the client.
 *
 * @param queue_id An existing queue identifier.
 * @return A new MidiQueue instance.
 */
MidiQueue*
MidiClient::useQueue(int queue_id)
{
    if (m_Queue != NULL) {
        delete m_Queue;
    }
    m_Queue = new MidiQueue(this, queue_id, this);
    return m_Queue;
}

/**
 * Create a new MidiQueue instance using a queue already existing in the
 * system, associating it to the client.
 *
 * @param name An existing queue name.
 * @return A new MidiQueue instance.
 */
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

/**
 * Associate an existing MidiQueue instance to the client.
 *
 * @param queue An existing MidiQueue.
 * @return The provided MidiQueue instance.
 */
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

/**
 * Get a list of the existing queues
 * @return a list of existing queues
 */
QList<int>
MidiClient::getAvailableQueues()
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

} /* namespace aseqmm */

