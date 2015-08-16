/*
    MIDI Sequencer C++ library
    Copyright (C) 2006-2015, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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
#include <QFile>
#include <QRegExp>
#include <QThread>
#include <QReadLocker>
#include <QWriteLocker>
#if defined(RTKIT_SUPPORT)
#include <QDBusConnection>
#include <QDBusInterface>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/resource.h>
#endif
#include <pthread.h>

#ifndef RLIMIT_RTTIME
#define RLIMIT_RTTIME 15
#endif

#ifndef SCHED_RESET_ON_FORK
#define SCHED_RESET_ON_FORK 0x40000000
#endif

#ifndef DEFAULT_INPUT_TIMEOUT
#define DEFAULT_INPUT_TIMEOUT 500
#endif

/**
 * @file alsaclient.cpp
 * Implementation of classes managing ALSA Sequencer clients
 */

/**
 * @class QObject
 * The QObject class is the base class of all Qt objects.
 * @see   http://qt-project.org/doc/qt-5/qobject.html
 */

/**
 * @class QThread
 * The QThread class provides platform-independent threads.
 * @see   http://qt-project.org/doc/qt-5/qthread.html
 */

namespace drumstick {

/**
@mainpage drumstick Documentation
@author Copyright &copy; 2009-2014 Pedro López-Cabanillas &lt;plcl AT users.sf.net&gt;
@date 2014-08-30
@version 1.0.0

This document is licensed under the Creative Commons Attribution-Share Alike 3.0 Unported License.
To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/

@section Abstract

This is the reference documentation for drumstick. This library is a set of C++ MIDI related classes,
using Qt5 objects, idioms and style.
ALSA sequencer provides software support for MIDI technology on Linux.

@see http://qt-project.org/doc/qt-5/index.html
@see http://www.alsa-project.org/alsa-doc/alsa-lib/seq.html
@see http://www.ics.com/design-patterns
@see http://www.midi.org/aboutmidi/tutorials.php

@section Disclaimer

This document is a work in progress, in a very early state. It will be always in
development. Please visit the drumstick web site to read the latest version.

@see http://drumstick.sourceforge.net

@section Introduction

For an introduction to design and programming with C++ and Qt, see the book
"An Introduction to Design Patterns in C++ with Qt" by by Alan Ezust and Paul
Ezust. It is available published on dead trees, and also
<a href="http://www.ics.com/design-patterns"> online</a>.

Here is how a simple program playing a note-on MIDI message using drumstick
looks like:

@code
#include <QApplication>
#include <drumstick.h>

int main(int argc, char **argv) {
    QApplication app(argc, argv, false);

    // create a client object on the heap
    drumstick::MidiClient *client = new drumstick::MidiClient;
    client->open();
    client->setClientName( "MyClient" );

    // create the port. Pointer is owned by the client instance
    drumstick::MidiPort *port = client->createPort();
    port->setPortName( "MyPort" );
    port->setCapability( SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ );
    port->setPortType( SND_SEQ_PORT_TYPE_MIDI_GENERIC );
    // subscribe the port to some other client:port
    port->subscribeTo( "20:0" ); // or "name:port", like in "KMidimon:0"

    // create an event object on the stack, to send a note on message
    drumstick::NoteOnEvent ev( 0, 66, 100 ); // (channel, note number, velocity)
    ev.setSource( port->getPortId() );
    ev.setSubscribers();   // deliver to all the connected ports
    ev.setDirect();        // not scheduled, deliver immediately
    client->output( &ev ); // or outputDirect() if you prefer not buffered
    client->drainOutput(); // flush the buffer

    // close and clean
    client->close();
    delete client;
    return 0;
}
@endcode

There are more examples in the source tree, under the utils/ directory, and
you can also see applications using this library, like kmetronome and kmidimon.

@see http://kmetronome.sourceforge.net
@see http://kmidimon.sourceforge.net
@see http://kmid2.sourceforge.net
@see http://vmpk.sourceforge.net

@section Acknowledgments
Parts of this documentation are copied from the ALSA library documentation,
whose authors are:
<ul>
<li>Jaroslav Kysela &lt;perex AT perex.cz&gt;</li>
<li>Abramo Bagnara &lt;abramo AT alsa-project.org&gt;</li>
<li>Takashi Iwai &lt;tiwai AT suse.de&gt;</li>
<li>Frank van de Pol &lt;fvdpol AT coil.demon.nl&gt;</li>
</ul>

@example drumgrid.cpp
Simple drum patterns
@include drumgrid.h

@example dumpmid.cpp
Print received sequencer events
@include dumpmid.h

@example playsmf.cpp
SMF playback, command line interface program
@include playsmf.h

@example guiplayer.cpp
SMF playback, graphic user interface program
@include guiplayer.h

@example buildsmf.cpp
SMF output from scratch
@include buildsmf.h

@example dumpsmf.cpp
SMF read and print
@include dumpsmf.h

@example dumpwrk.cpp
Cakewalk WRK file parse and print
@include dumpwrk.h

@example dumpove.cpp
Overture OVE file parse and print
@include dumpove.h

@example metronome.cpp
Simple command line MIDI metronome
@include metronome.h

@example sysinfo.cpp
Prints information about the ALSA sequencer subsystem

@example testevents.cpp
SequencerEvents test
@include testevents.h

@example timertest.cpp
ALSA Timers test
@include timertest.h

@example vpiano.cpp
A Virtual Piano Keyboard GUI application. See another one at http://vmpk.sf.net
@include vpiano.h

*/

/**
 * @addtogroup ALSAClient
 * @{
 *
 * ALSA clients are any entities using ALSA sequencer services. A client
 * may be an application or a device driver for an external MIDI port, like
 * USB MIDI devices or the MIDI/game ports of some sound cards. This library
 * allows to easily create applications managing ALSA clients.
 *
 * ALSA clients are also file descriptors representing a sequencer device,
 * that must be opened before reading or writing MIDI events. When the client
 * is opened, it is given some handle and a number identifying it to other
 * clients in the system. You can also provide a name for it.
 *
 * Each ALSA sequencer client can have several ports attached. The ports can be
 * readable or writable, and can be subscribed in pairs: one readable port to
 * one writable port. The subscriptions can be made and queried by external
 * applications, like "aconnect" or "qjackctl".
 *
 * SystemInfo is an auxiliary class to query several system capabilities.
 *
 * The PoolInfo class represents a container to query and change some values
 * for the kernel memory pool assigned to an ALSA client.
 *
 * The ClientInfo class is another container to query and change properties of
 * the MidiClient itself.
 *
 * The SequencerEventHandler abstract class is used to define an interface
 * that other class can implement to receive sequencer events. It is one of the
 * three methods of delivering input events offered by the library.
 *
 * @section EventInput Input
 * MidiClient uses a separate thread to receive events from the ALSA sequencer.
 * The input thread can be started and stopped using the methods
 * MidiClient::startSequencerInput() and MidiClient::stopSequencerInput().
 * It is necessary to have this thread in mind when using this library to read
 * events. There are three delivering methods of input events:
 * <ul>
 * <li>A Callback method. To use this method, you must derive a class from
 * SequencerEventHandler, overriding the method
 * SequencerEventHandler::handleSequencerEvent() to provide your own event
 * processing code. You must give a handler instance pointer to
 * the client using MidiClient::setHandler().</li>
 * <li>Using QEvent listeners. To use this method, you must have one or more
 * classes derived from QObject overriding the method QObject::customEvent().
 * You must also use the method MidiClient::addListener() to add such objects
 * to the client's listeners list, and MidiClient::setEventsEnabled().</li>
 * <li>The third method involves signals and slots. Whenever a sequencer event
 * is received, a signal MidiClient::eventReceived() is emitted, that can be
 * connected to your own supplied slot(s) to process it.
 * </ul>
 * The selected method depends only on your requirements and your preferences.
 * <ul>
 * <li>The Callback method is preferred for real-time usage because the handler
 * receives the events without any delay, but at the same time you must
 * avoid calling methods of any GUI widgets within the handler. Instead,
 * you can create QEvents and call QObject::postEvent() to notify the GUI.</li>
 * <li>Inside QObject::eventReceiver() you can collect QEvents and call
 * any method you want, but the events are not delivered in real-time. Instead,
 * they are enqueued and dispatched by the main application's event loop.</li>
 * <li>The signals/slots method can be real-time or queued, depending on the
 * last parameter of QObject::connect(). If it is Qt::DirectConnection, the signal
 * is delivered in real-time, and the same rule about avoiding calls to any
 * GUI widgets methods apply. If it is Qt::QueuedConnection, then the signal is
 * enqueued using the application's event loop, and it is safe to call any GUI
 * methods in this case.</li>
 * </ul>
 * Whichever method you select, it excludes the other methods for the same
 * program. A callback takes precedence over the others. If it is not set, then
 * the events are sent if MidiClient::setEventsEnabled() is called.
 * If neither a callback handler is set nor events are enabled, then the signal
 * is emitted. In any case, the event pointer must be deleted by the receiver
 * method.
 *
 * @see http://qt-project.org/doc/qt-5/threads-reentrancy.html
 *
 * @section EventOutput Output
 *
 * The methods to send a single event to the ALSA sequencer are:
 * <ul>
 * <li>MidiClient::output() using the library buffer, automatically flushed.</li>
 * <li>MidiClient::outputBuffer() using the library buffer. Not flushed automatically.</li>
 * <li>MidiClient::outputDirect() not using the library buffer.</li>
 * </ul>
 * The two first methods usually require a call to MidiClient::drainOutput() to
 * flush the ALSA library output buffer. The third one bypasses the buffer, and
 * doesn't require the call to MidiClient::drainOutput(). Note that the buffer
 * can be automatically drained by the first method when it becomes full.
 *
 * After being dispatched to the ALSA Sequencer, the events can be scheduled at
 * some time in the future, or immediately. This depends on the following
 * methods of the SequencerEvent class:
 * <ul>
 * <li>SequencerEvent::setDirect() not scheduled</li>
 * <li>SequencerEvent::scheduleTick() scheduled in musical time (ticks)</li>
 * <li>SequencerEvent::scheduleReal() scheduled in clock time (seconds)</li>
 * </ul>
 *
 * When you need to schedule a lot of events, for instance reproducing
 * a Standard MIDI File (SMF) or a MIDI sequence, you may want to use the
 * abstract class SequencerOutputThread.
 *
 * @section Memory
 *
 * There are two memory issues: the memory pool belongs to the kernel sequencer,
 * and can be managed by the class PoolInfo and the methods
 * MidiClient::getPoolInfo() and MidiClient::setPoolInfo(). The library buffer
 * can be controlled using the methods MidiClient::getOutputBufferSize() and
 * MidiClient::setOutputBufferSize() as well as MidiClient::getInputBufferSize()
 * and MidiClient::setInputBufferSize().
 *
 * @see http://www.alsa-project.org/alsa-doc/alsa-lib/group___seq_client.html
 * @}
 */

/**
 * This class manages event input from the ALSA sequencer.
 */
class MidiClient::SequencerInputThread: public QThread
{
public:
    SequencerInputThread(MidiClient *seq, int timeout)
        : QThread(),
        m_MidiClient(seq),
        m_Wait(timeout),
        m_Stopped(false),
        m_RealTime(true) {}
    virtual ~SequencerInputThread() {}
    virtual void run();
    bool stopped();
    void stop();
    void setRealtimePriority();

    MidiClient *m_MidiClient;
    int m_Wait;
    bool m_Stopped;
    bool m_RealTime;
    QReadWriteLock m_mutex;
};

class MidiClient::MidiClientPrivate
{
public:
    MidiClientPrivate() :
        m_eventsEnabled(false),
        m_BlockMode(false),
        m_NeedRefreshClientList(true),
        m_OpenMode(SND_SEQ_OPEN_DUPLEX),
        m_DeviceName("default"),
        m_SeqHandle(0),
        m_Thread(0),
        m_Queue(0),
        m_handler(0)
    { }

    bool m_eventsEnabled;
    bool m_BlockMode;
    bool m_NeedRefreshClientList;
    int  m_OpenMode;
    QString m_DeviceName;
    snd_seq_t* m_SeqHandle;
    QPointer<SequencerInputThread> m_Thread;
    QPointer<MidiQueue> m_Queue;
    SequencerEventHandler* m_handler;

    ClientInfo m_Info;
    ClientInfoList m_ClientList;
    MidiPortList m_Ports;
    PortInfoList m_OutputsAvail;
    PortInfoList m_InputsAvail;
    QObjectList m_listeners;
    SystemInfo m_sysInfo;
    PoolInfo m_poolInfo;
};

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
 * @param parent The optional parent object
 * @return a MidiClient instance
 */
MidiClient::MidiClient( QObject* parent ) :
    QObject(parent),
    d(new MidiClientPrivate)
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
    if (d->m_Queue != 0)
        delete d->m_Queue;
    close();
    freeClients();
    if (d->m_Thread != 0)
        delete d->m_Thread;
    delete d;
}


/**
 * Returns the sequencer handler managed by ALSA
 */
snd_seq_t*
MidiClient::getHandle()
{
    return d->m_SeqHandle;
}

/**
 * Returns true if the sequencer is opened
 */
bool MidiClient::isOpened()
{
    return (d->m_SeqHandle != NULL);
}

/**
 * Returns the name of the sequencer device
 */
QString MidiClient::getDeviceName()
{
    return d->m_DeviceName;
}

/**
 * Returns the last open mode used in open()
 */
int MidiClient::getOpenMode()
{
    return d->m_OpenMode;
}

/**
 * Returns the last block mode used in open()
 */
bool MidiClient::getBlockMode()
{
    return d->m_BlockMode;
}

/**
 * Returns true if the events mode of delivery has been enabled
 */
bool MidiClient::getEventsEnabled() const
{
    return d->m_eventsEnabled;
}

/**
 * Sets a sequencer event handler enabling the callback delivery mode
 */
void MidiClient::setHandler(SequencerEventHandler* handler)
{
    d->m_handler = handler;
}


/**
 * Enables real-time priority for the MIDI input thread. The system needs either
 * RLIMIT_RTPRIO or RealtimeKit. First RLIMIT_RTPRIO is tried, and if this
 * method fails, RealtimeKit is used.
 *
 * @param enable real-time priority enabled
 * @since 0.5.0
 */
void MidiClient::setRealTimeInput(bool enable)
{
    if (d->m_Thread == 0) {
        d->m_Thread = new SequencerInputThread(this, DEFAULT_INPUT_TIMEOUT);
        d->m_Thread->m_RealTime = enable;
    }
}

/**
 * Return the real-time priority setting for the MIDI input thread.
 * @return true if the real-time priority is enabled
 * @since 0.5.0
 */
bool MidiClient::realTimeInputEnabled()
{
    if (d->m_Thread == 0)
        return true;
    return d->m_Thread->m_RealTime;
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
    CHECK_ERROR( snd_seq_open( &d->m_SeqHandle, deviceName.toLocal8Bit().data(),
                              openMode, blockMode ? 0 : SND_SEQ_NONBLOCK ) );
    CHECK_WARNING( snd_seq_get_client_info( d->m_SeqHandle, d->m_Info.m_Info ) );
    d->m_DeviceName = deviceName;
    d->m_OpenMode = openMode;
    d->m_BlockMode = blockMode;
}

/**
 * Open the sequencer device, providing a configuration object pointer.
 *
 * This method is like open() except that a configuration is explicitly
 * provided. After a successful open, an event with SND_SEQ_EVENT_CLIENT_START
 * type is broadcasted from the announce port.
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
    CHECK_ERROR( snd_seq_open_lconf( &d->m_SeqHandle,
                                     deviceName.toLocal8Bit().data(),
                                     openMode,
                                     blockMode ? 0 : SND_SEQ_NONBLOCK,
                                     conf ));
    CHECK_WARNING( snd_seq_get_client_info(d->m_SeqHandle, d->m_Info.m_Info));
    d->m_DeviceName = deviceName;
    d->m_OpenMode = openMode;
    d->m_BlockMode = blockMode;
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
    if (d->m_SeqHandle != NULL) {
        stopSequencerInput();
        CHECK_WARNING(snd_seq_close(d->m_SeqHandle));
        d->m_SeqHandle = NULL;
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
    return snd_seq_get_output_buffer_size(d->m_SeqHandle);
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
        CHECK_WARNING(snd_seq_set_output_buffer_size(d->m_SeqHandle, newSize));
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
    return snd_seq_get_input_buffer_size(d->m_SeqHandle);
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
        CHECK_WARNING(snd_seq_set_input_buffer_size(d->m_SeqHandle, newSize));
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
    if (d->m_BlockMode != newValue)
    {
        d->m_BlockMode = newValue;
        if (d->m_SeqHandle != NULL)
        {
            CHECK_WARNING(snd_seq_nonblock(d->m_SeqHandle, d->m_BlockMode ? 0 : 1));
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
    return CHECK_WARNING(snd_seq_client_id(d->m_SeqHandle));
}

/**
 * Returns the type snd_seq_type_t of the given sequencer handle.
 * @return the type snd_seq_type_t of the given sequencer handle.
 */
snd_seq_type_t
MidiClient::getSequencerType()
{
    return snd_seq_type(d->m_SeqHandle);
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
        err = snd_seq_event_input(d->m_SeqHandle, &evp);
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
                d->m_NeedRefreshClientList = true;
                break;

            case SND_SEQ_EVENT_CLIENT_CHANGE:
            case SND_SEQ_EVENT_CLIENT_EXIT:
            case SND_SEQ_EVENT_CLIENT_START:
                event = new ClientEvent(evp);
                d->m_NeedRefreshClientList = true;
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
            if (d->m_handler != NULL) {
                d->m_handler->handleSequencerEvent(event->clone());
            } else {
                // second, process the event listeners
                if (d->m_eventsEnabled) {
                   QObjectList::Iterator it;
                    for(it=d->m_listeners.begin(); it!=d->m_listeners.end(); ++it) {
                        QObject* sub = (*it);
                        QCoreApplication::postEvent(sub, event->clone());
                    }
                } else {
                    // finally, process signals
                    emit eventReceived(event->clone());
                }
            }
            delete event;
        }
    }
    while (snd_seq_event_input_pending(d->m_SeqHandle, 0) > 0);
}

/**
 * Starts reading events from the ALSA sequencer.
 */
void
MidiClient::startSequencerInput()
{
    if (d->m_Thread == 0) {
        d->m_Thread = new SequencerInputThread(this, DEFAULT_INPUT_TIMEOUT);
    }
    d->m_Thread->start( d->m_Thread->m_RealTime ?
            QThread::TimeCriticalPriority : QThread::InheritPriority );
}

/**
 * Stops reading events from the ALSA sequencer.
 */
void
MidiClient::stopSequencerInput()
{
    int counter = 0;
    if (d->m_Thread != 0) {
        if (d->m_Thread->isRunning()) {
            d->m_Thread->stop();
            while (!d->m_Thread->wait(500) && (counter < 10)) {
                counter++;
            }
            if (!d->m_Thread->isFinished()) {
                d->m_Thread->terminate();
            }
        }
        delete d->m_Thread;
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
    while (snd_seq_query_next_client(d->m_SeqHandle, cInfo.m_Info) >= 0) {
        cInfo.readPorts(this);
        d->m_ClientList.append(cInfo);
    }
    d->m_NeedRefreshClientList = false;
}

/**
 * Releases the list of ALSA sequencer's clients.
 */
void
MidiClient::freeClients()
{
    d->m_ClientList.clear();
}

/**
 * Gets the list of clients from the ALSA sequencer.
 * @return the list of clients.
 */
ClientInfoList
MidiClient::getAvailableClients()
{
    if (d->m_NeedRefreshClientList)
        readClients();
    ClientInfoList lst = d->m_ClientList; // copy
    return lst;
}

/**
 * Gets the ClientInfo object holding data about this client.
 * @return the ClientInfo object representing this client.
 */
ClientInfo&
MidiClient::getThisClientInfo()
{
    snd_seq_get_client_info(d->m_SeqHandle, d->m_Info.m_Info);
    return d->m_Info;
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
    d->m_Info = val;
    snd_seq_set_client_info(d->m_SeqHandle, d->m_Info.m_Info);
}

/**
 * This internal method applies the ClientInfo data to the ALSA sequencer client
 */
void
MidiClient::applyClientInfo()
{
    if (d->m_SeqHandle != NULL) {
        snd_seq_set_client_info(d->m_SeqHandle, d->m_Info.m_Info);
    }
}

/**
 * Gets the client's public name
 * @return The client's name
 */
QString
MidiClient::getClientName()
{
    return d->m_Info.getName();
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
    if (d->m_NeedRefreshClientList)
        readClients();
    for (it = d->m_ClientList.begin(); it != d->m_ClientList.end(); ++it) {
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
    if (newName != d->m_Info.getName()) {
        d->m_Info.setName(newName);
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
    return d->m_Ports;
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
    if (d->m_SeqHandle != NULL) {
        CHECK_ERROR(snd_seq_create_port(d->m_SeqHandle, port->m_Info.m_Info));
        d->m_Ports.push_back(port);
    }
}

/**
 * Detach a MidiPort instance from this client
 * @param port The MidiPort to be detached
 */
void
MidiClient::portDetach(MidiPort* port)
{
    if (d->m_SeqHandle != NULL) {
        if(port->getPortInfo()->getClient() == getClientId())
        {
            return;
        }
        CHECK_ERROR(snd_seq_delete_port(d->m_SeqHandle, port->getPortInfo()->getPort()));
        port->setMidiClient(NULL);

        MidiPortList::iterator it;
        for(it = d->m_Ports.begin(); it != d->m_Ports.end(); ++it)
        {
            if ((*it)->getPortInfo()->getPort() == port->getPortInfo()->getPort())
            {
                d->m_Ports.erase(it);
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
    if (d->m_SeqHandle != NULL) {
        MidiPortList::iterator it;
        for (it = d->m_Ports.begin(); it != d->m_Ports.end(); ++it) {
            CHECK_ERROR(snd_seq_delete_port(d->m_SeqHandle, (*it)->getPortInfo()->getPort()));
            (*it)->setMidiClient(NULL);
            d->m_Ports.erase(it);
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
    snd_seq_set_client_event_filter(d->m_SeqHandle, evtype);
}

/**
 * Gets the broadcast filter usage of the client.
 *
 * @return The broadcast filter.
 */
bool
MidiClient::getBroadcastFilter()
{
    return d->m_Info.getBroadcastFilter();
}

/**
 * Sets the broadcast filter usage of the client.
 *
 * @param newValue The broadcast filter.
 */
void
MidiClient::setBroadcastFilter(bool newValue)
{
    d->m_Info.setBroadcastFilter(newValue);
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
    return d->m_Info.getErrorBounce();
}

/**
 * Sets the error-bounce usage of the client.
 *
 * @param newValue The error-bounce usage.
 */
void
MidiClient::setErrorBounce(bool newValue)
{
    d->m_Info.setErrorBounce(newValue);
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
 * event can be delivered.
 * @param timeout The maximum time to wait in synchronous mode.
 */
void
MidiClient::output(SequencerEvent* ev, bool async, int timeout)
{
    int npfds;
    pollfd* pfds;
    if (async) {
        CHECK_WARNING(snd_seq_event_output(d->m_SeqHandle, ev->getHandle()));
    } else {
        npfds = snd_seq_poll_descriptors_count(d->m_SeqHandle, POLLOUT);
        pfds = (pollfd*) alloca(npfds * sizeof(pollfd));
        snd_seq_poll_descriptors(d->m_SeqHandle, pfds, npfds, POLLOUT);
        while (snd_seq_event_output(d->m_SeqHandle, ev->getHandle()) < 0)
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
 * event is delivered to the sequencer.
 * @param timeout The maximum time to wait in synchronous mode.
 */
void MidiClient::outputDirect(SequencerEvent* ev, bool async, int timeout)
{
    int npfds;
    pollfd* pfds;
    if (async) {
        CHECK_WARNING(snd_seq_event_output_direct(d->m_SeqHandle, ev->getHandle()));
    } else {
        npfds = snd_seq_poll_descriptors_count(d->m_SeqHandle, POLLOUT);
        pfds = (pollfd*) alloca(npfds * sizeof(pollfd));
        snd_seq_poll_descriptors(d->m_SeqHandle, pfds, npfds, POLLOUT);
        while (snd_seq_event_output_direct(d->m_SeqHandle, ev->getHandle()) < 0)
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
 */
void
MidiClient::outputBuffer(SequencerEvent* ev)
{
    CHECK_WARNING(snd_seq_event_output_buffer(d->m_SeqHandle, ev->getHandle()));
}

/**
 * Drain the library output buffer.
 *
 * This function drains all pending events on the output buffer. The function
 * returns immediately after the events are sent to the queues regardless
 * whether the events are processed or not.
 *
 * @param async Use asynchronous mode. If false, this call will block until the
 * buffer can be flushed.
 * @param timeout The maximum time to wait in synchronous mode.
 */
void MidiClient::drainOutput(bool async, int timeout)
{
    int npfds;
    pollfd* pfds;
    if (async) {
        CHECK_WARNING(snd_seq_drain_output(d->m_SeqHandle));
    } else {
        npfds = snd_seq_poll_descriptors_count(d->m_SeqHandle, POLLOUT);
        pfds = (pollfd*) alloca(npfds * sizeof(pollfd));
        snd_seq_poll_descriptors(d->m_SeqHandle, pfds, npfds, POLLOUT);
        while (snd_seq_drain_output(d->m_SeqHandle) < 0)
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
    snd_seq_sync_output_queue(d->m_SeqHandle);
}

/**
 * Get the MidiQueue instance associated to this client.
 * If the client is not associated to a MidiQueue, one is created.
 * @return A MidiQueue instance pointer
 */
MidiQueue*
MidiClient::getQueue()
{
    if (d->m_Queue == NULL) {
        createQueue();
    }
    return d->m_Queue;
}

/**
 * Create and return a new MidiQueue associated to this client.
 * @return A new MidiQueue instance.
 */
MidiQueue*
MidiClient::createQueue()
{
    if (d->m_Queue != NULL) {
        delete d->m_Queue;
    }
    d->m_Queue = new MidiQueue(this, this);
    return d->m_Queue;
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
    if (d->m_Queue != NULL) {
        delete d->m_Queue;
    }
    d->m_Queue = new MidiQueue(this, queueName, this);
    return d->m_Queue;
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
    if (d->m_Queue != NULL) {
        delete d->m_Queue;
    }
    d->m_Queue = new MidiQueue(this, queue_id, this);
    return d->m_Queue;
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
    if (d->m_Queue != NULL) {
        delete d->m_Queue;
    }
    int queue_id = getQueueId(name);
    if ( queue_id >= 0) {
       d->m_Queue = new MidiQueue(this, queue_id, this);
    }
    return d->m_Queue;
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
    if (d->m_Queue != NULL) {
        delete d->m_Queue;
    }
    queue->setParent(this);
    d->m_Queue = queue;
    return d->m_Queue;
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
        err = snd_seq_get_queue_info(d->m_SeqHandle, q, qinfo);
        if (err == 0) {
            queues.append(q);
        }
    }
    return queues;
}

/**
 * Gets a list of the available user ports in the system, filtered by the given
 * bitmap of desired capabilities.
 *
 * @param filter A bitmap of capabilities.
 * @return A filtered list of the available ports in the system.
 */
PortInfoList
MidiClient::filterPorts(unsigned int filter)
{
    PortInfoList result;
    ClientInfoList::ConstIterator itc;
    PortInfoList::ConstIterator itp;

    if (d->m_NeedRefreshClientList)
        readClients();

    for (itc = d->m_ClientList.constBegin(); itc != d->m_ClientList.constEnd(); ++itc) {
        ClientInfo ci = (*itc);
        if ((ci.getClientId() == SND_SEQ_CLIENT_SYSTEM) ||
            (ci.getClientId() == d->m_Info.getClientId()))
            continue;
        PortInfoList lstPorts = ci.getPorts();
        for(itp = lstPorts.constBegin(); itp != lstPorts.constEnd(); ++itp) {
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

/**
 * Update the internal lists of user ports.
 */
void
MidiClient::updateAvailablePorts()
{
    d->m_InputsAvail.clear();
    d->m_OutputsAvail.clear();
    d->m_InputsAvail = filterPorts( SND_SEQ_PORT_CAP_READ |
                                 SND_SEQ_PORT_CAP_SUBS_READ );
    d->m_OutputsAvail = filterPorts( SND_SEQ_PORT_CAP_WRITE |
                                  SND_SEQ_PORT_CAP_SUBS_WRITE );
}

/**
 * Gets the available user input ports in the system.
 * @return The list of available input ports.
 */
PortInfoList
MidiClient::getAvailableInputs()
{
    d->m_NeedRefreshClientList = true;
    updateAvailablePorts();
    return d->m_InputsAvail;
}

/**
 * Gets the available user output ports in the system.
 * @return The list of available output ports.
 */
PortInfoList
MidiClient::getAvailableOutputs()
{
    d->m_NeedRefreshClientList = true;
    updateAvailablePorts();
    return d->m_OutputsAvail;
}

/**
 * Adds a QObject to the listeners list. This object should override the method
 * QObject::customEvent() to receive SequencerEvent instances.
 * @param listener A QObject listener to be notified of received events.
 * @see removeListener(), setEventsEnabled()
 */
void
MidiClient::addListener(QObject* listener)
{
    d->m_listeners.append(listener);
}

/**
 * Removes a QObject listener from the listeners list.
 * @param listener listener A QObject listener to be removed of received events.
 * @see addListener(), setEventsEnabled()
 */
void
MidiClient::removeListener(QObject* listener)
{
    d->m_listeners.removeAll(listener);
}

/**
 * Enables the notification of received SequencerEvent instances to the listeners
 * registered with addListener()
 * @param bEnabled The new state of the events delivering mode.
 * @see addListener(), removeListener(), setEventsEnabled()
 */
void
MidiClient::setEventsEnabled(bool bEnabled)
{
    if (bEnabled != d->m_eventsEnabled) {
        d->m_eventsEnabled = bEnabled;
    }
}

/**
 * Gets a SystemInfo instance with the updated state of the system.
 * @return The updated system info.
 */
SystemInfo&
MidiClient::getSystemInfo()
{
    snd_seq_system_info(d->m_SeqHandle, d->m_sysInfo.m_Info);
    return d->m_sysInfo;
}

/**
 * Gets a PoolInfo instance with an updated state of the client memory pool
 * @return The updated memory pool state.
 */
PoolInfo&
MidiClient::getPoolInfo()
{
    snd_seq_get_client_pool(d->m_SeqHandle, d->m_poolInfo.m_Info);
    return d->m_poolInfo;
}

/**
 * Applies (updates) the client's PoolInfo data into the system.
 * @param info The PoolInfo reference to be applied to the client.
 */
void
MidiClient::setPoolInfo(const PoolInfo& info)
{
    d->m_poolInfo = info;
    CHECK_WARNING(snd_seq_set_client_pool(d->m_SeqHandle, d->m_poolInfo.m_Info));
}

/**
 * Resets the client input pool.
 * @see dropInput()
 */
void
MidiClient::resetPoolInput()
{
    CHECK_WARNING(snd_seq_reset_pool_input(d->m_SeqHandle));
}

/**
 * Resets the client output pool.
 * @see dropOutput()
 */
void
MidiClient::resetPoolOutput()
{
    CHECK_WARNING(snd_seq_reset_pool_output(d->m_SeqHandle));
}

/**
 * Sets the size of the client's input pool.
 * @param size The new size
 */
void
MidiClient::setPoolInput(int size)
{
    CHECK_WARNING(snd_seq_set_client_pool_input(d->m_SeqHandle, size));
}

/**
 * Sets the size of the client's output pool.
 * @param size The new size
 */
void
MidiClient::setPoolOutput(int size)
{
    CHECK_WARNING(snd_seq_set_client_pool_output(d->m_SeqHandle, size));
}

/**
 * Sets the room size of the client's output pool.
 * @param size The new size
 */
void
MidiClient::setPoolOutputRoom(int size)
{
    CHECK_WARNING(snd_seq_set_client_pool_output_room(d->m_SeqHandle, size));
}

/**
 * Clears the client's input buffer and and remove events in sequencer queue.
 * @see resetPoolInput()
 */
void
MidiClient::dropInput()
{
    CHECK_WARNING(snd_seq_drop_input(d->m_SeqHandle));
}

/**
 * Remove all events on user-space input buffer.
 * @see dropInput()
 */
void
MidiClient::dropInputBuffer()
{
    CHECK_WARNING(snd_seq_drop_input_buffer(d->m_SeqHandle));
}

/**
 * Clears the client's output buffer and and remove events in sequencer queue.
 *
 * This method removes all events on both user-space output buffer and output
 * memory pool on kernel.
 * @see resetPoolOutput()
 */
void
MidiClient::dropOutput()
{
    CHECK_WARNING(snd_seq_drop_output(d->m_SeqHandle));
}

/**
 * Removes all events on the library output buffer.
 *
 * Removes all events on the user-space output buffer. Unlike dropOutput(), this
 * method doesn't remove events on the client's output memory pool.
 * @see dropOutput()
 */
void
MidiClient::dropOutputBuffer()
{
    CHECK_WARNING(snd_seq_drop_output_buffer(d->m_SeqHandle));
}

/**
 * Removes events on input/output buffers and pools.
 * Removes matching events with the given condition from input/output buffers
 * and pools. The removal condition is specified in the spec argument.
 * @param spec A RemoveEvents instance specifying the removal condition.
 */
void
MidiClient::removeEvents(const RemoveEvents* spec)
{
    CHECK_WARNING(snd_seq_remove_events(d->m_SeqHandle, spec->m_Info));
}

/**
 * Extracts (and removes) the first event in the output buffer.
 * @return The extracted event.
 */
SequencerEvent*
MidiClient::extractOutput()
{
    snd_seq_event_t* ev;
    if (CHECK_WARNING(snd_seq_extract_output(d->m_SeqHandle, &ev) == 0)) {
        return new SequencerEvent(ev);
    }
    return NULL;
}

/**
 * Returns the size of pending events on the output buffer.
 *
 * @return The size of pending events.
 */
int
MidiClient::outputPending()
{
    return snd_seq_event_output_pending(d->m_SeqHandle);
}

/**
 * Gets the size of the events on the input buffer.
 *
 * If there are events remaining on the user-space input buffer, this method
 * returns the total size of events on it. If the argument is true, this method
 * checks the presence of events on the sequencer FIFO, and when events exist
 * they are transferred to the input buffer, and the number of received events
 * are returned. If the argument is false and no events remain on the input
 * buffer, this method simply returns zero.
 *
 * @param fetch Check and fetch the sequencer input pool.
 * @return The size in bytes of the remaining input events on the buffer.
 */
int
MidiClient::inputPending(bool fetch)
{
    return snd_seq_event_input_pending(d->m_SeqHandle, fetch ? 1 : 0);
}

/**
 * Gets the queue's numeric identifier corresponding to the provided name.
 *
 * @param name The name string to query.
 * @return The number of the matching queue.
 */
int
MidiClient::getQueueId(const QString& name)
{
    return snd_seq_query_named_queue(d->m_SeqHandle, name.toLocal8Bit().data());
}

/**
 * Returns the number of poll descriptors.
 * @param events Poll events to be checked (POLLIN and POLLOUT).
 * @return The number of poll descriptors.
 */
int
MidiClient::getPollDescriptorsCount(short events)
{
    return snd_seq_poll_descriptors_count(d->m_SeqHandle, events);
}

/**
 * Get poll descriptors.
 *
 * Get poll descriptors assigned to the sequencer handle. Since a sequencer
 * handle can duplex streams, you need to set which direction(s) is/are polled
 * in events argument. When POLLIN bit is specified, the incoming events to the
 * ports are checked.
 *
 * @param pfds  Array of poll descriptors
 * @param space Space in the poll descriptor array
 * @param events Polling events to be checked (POLLIN and POLLOUT)
 * @return Count of filled descriptors
 */
int
MidiClient::pollDescriptors( struct pollfd *pfds, unsigned int space,
                             short events )
{
    return snd_seq_poll_descriptors(d->m_SeqHandle, pfds, space, events);
}

/**
 * Gets the number of returned events from poll descriptors
 * @param pfds Array of poll descriptors.
 * @param nfds Count of poll descriptors.
 * @return Number of returned events.
 */
unsigned short
MidiClient::pollDescriptorsRevents(struct pollfd *pfds, unsigned int nfds)
{
    unsigned short revents;
    CHECK_WARNING( snd_seq_poll_descriptors_revents( d->m_SeqHandle,
                                                     pfds, nfds,
                                                     &revents ));
    return revents;
}

/**
 * Gets the internal sequencer device name
 * @return The device name.
 */
const char *
MidiClient::_getDeviceName()
{
    return snd_seq_name(d->m_SeqHandle);
}

/**
 * Sets the client name
 * @param name The new client name.
 */
void
MidiClient::_setClientName(const char *name)
{
    CHECK_WARNING(snd_seq_set_client_name(d->m_SeqHandle, name));
}

/**
 * Create an ALSA sequencer port, without using MidiPort.
 * @param name The name of the new port.
 * @param caps The new port capabilities.
 * @param type The type of the new port.
 * @return The port numeric identifier.
 */
int
MidiClient::createSimplePort( const char *name,
                              unsigned int caps,
                              unsigned int type )
{
    return CHECK_WARNING( snd_seq_create_simple_port( d->m_SeqHandle,
                                                      name, caps, type ));
}

/**
 * Remove an ALSA sequencer port.
 * @param port The numeric identifier of the port.
 */
void
MidiClient::deleteSimplePort(int port)
{
    CHECK_WARNING( snd_seq_delete_simple_port( d->m_SeqHandle, port ));
}

/**
 * Subscribe one port from another arbitrary sequencer client:port.
 * @param myport The number of the internal port.
 * @param client The external client's identifier.
 * @param port The external port's identifier.
 */
void
MidiClient::connectFrom(int myport, int client, int port)
{
    CHECK_WARNING( snd_seq_connect_from(d->m_SeqHandle, myport, client, port ));
}

/**
 * Subscribe one port to another arbitrary sequencer client:port.
 * @param myport The number of the internal port.
 * @param client The external client's identifier.
 * @param port The external port's identifier.
 */
void
MidiClient::connectTo(int myport, int client, int port)
{
    CHECK_WARNING( snd_seq_connect_to(d->m_SeqHandle, myport, client, port ));
}

/**
 * Unsubscribe one port from another arbitrary sequencer client:port.
 * @param myport The number of the internal port.
 * @param client The external client's identifier.
 * @param port The external port's identifier.
 */
void
MidiClient::disconnectFrom(int myport, int client, int port)
{
    CHECK_WARNING( snd_seq_disconnect_from(d->m_SeqHandle, myport, client, port ));
}

/**
 * Unsubscribe one port to another arbitrary sequencer client:port.
 * @param myport The number of the internal port.
 * @param client The external client's identifier.
 * @param port The external port's identifier.
 */
void
MidiClient::disconnectTo(int myport, int client, int port)
{
    CHECK_WARNING( snd_seq_disconnect_to(d->m_SeqHandle, myport, client, port ));
}

/**
 * Parse a text address representation, returning an ALSA address record.
 *
 * This function can be used as a replacement of the standard ALSA function
 * snd_seq_parse_address().
 *
 * @param straddr source text address representation
 * @param addr returned ALSA address record
 * @return true if the text address was successfully parsed
 * @since 0.3.1
 */
bool
MidiClient::parseAddress( const QString& straddr, snd_seq_addr& addr )
{
    bool ok(false);
    QString testClient, testPort;
    ClientInfoList::ConstIterator cit;
    int pos = straddr.indexOf(':');
    if (pos > -1) {
        testClient = straddr.left(pos);
        testPort = straddr.mid(pos+1);
    } else {
        testClient = straddr;
        testPort = '0';
    }
    addr.client = testClient.toInt(&ok);
    if (ok)
        addr.port = testPort.toInt(&ok);
    if (!ok) {
        if (d->m_NeedRefreshClientList)
            readClients();
        for ( cit = d->m_ClientList.constBegin();
              cit != d->m_ClientList.constEnd(); ++cit ) {
            ClientInfo ci = *cit;
            if (testClient.compare(ci.getName(), Qt::CaseInsensitive) == 0) {
                addr.client = ci.getClientId();
                addr.port = testPort.toInt(&ok);
                return ok;
            }
        }
    }
    return ok;
}

/**
 * Returns true or false depending on the input thread state.
 * @return true if the input thread is stopped.
 */
bool
MidiClient::SequencerInputThread::stopped()
{
    QReadLocker locker(&m_mutex);
    return m_Stopped;
}

/**
 * Stops the input thread.
 */
void
MidiClient::SequencerInputThread::stop()
{
    QWriteLocker locker(&m_mutex);
    m_Stopped = true;
}

#if defined(RTKIT_SUPPORT)
static pid_t _gettid(void) {
    return (pid_t) ::syscall(SYS_gettid);
}
#endif

void
MidiClient::SequencerInputThread::setRealtimePriority()
{
    struct sched_param p;
    int rt, policy = SCHED_RR | SCHED_RESET_ON_FORK;
    quint32 priority = 6;
#if defined(RTKIT_SUPPORT)
    bool ok;
    quint32 max_prio;
    quint64 thread;
    struct rlimit old_limit, new_limit;
    long long max_rttime;
#endif

    ::memset(&p, 0, sizeof(p));
    p.sched_priority = priority;
    rt = ::pthread_setschedparam(::pthread_self(), policy, &p);
    if (rt != 0) {
#if defined(RTKIT_SUPPORT)
        const QString rtkit_service =
                QLatin1String("org.freedesktop.RealtimeKit1");
        const QString rtkit_path =
                QLatin1String("/org/freedesktop/RealtimeKit1");
        const QString rtkit_iface = rtkit_service;
        thread = _gettid();
        QDBusConnection bus = QDBusConnection::systemBus();
        QDBusInterface realtimeKit(rtkit_service, rtkit_path, rtkit_iface, bus);
        QVariant maxRTPrio = realtimeKit.property("MaxRealtimePriority");
        max_prio = maxRTPrio.toUInt(&ok);
        if (!ok) {
            qWarning() << "invalid property RealtimeKit.MaxRealtimePriority";
            return;
        }
        if (priority > max_prio)
            priority = max_prio;
        QVariant maxRTNSec = realtimeKit.property("RTTimeNSecMax");
        max_rttime = maxRTNSec.toLongLong(&ok);
        if (!ok || max_rttime < 0) {
            qWarning() << "invalid property RealtimeKit.RTTimeNSecMax";
            return;
        }
        new_limit.rlim_cur = new_limit.rlim_max = max_rttime;
        rt = ::getrlimit(RLIMIT_RTTIME, &old_limit);
        if (rt < 0) {
            qWarning() << "getrlimit() failed. err=" << rt << ::strerror(rt);
            return;
        }
        rt = ::setrlimit(RLIMIT_RTTIME, &new_limit);
        if ( rt < 0) {
            qWarning() << "setrlimit() failed, err=" << rt << ::strerror(rt);
            return;
        }
        QDBusMessage reply = realtimeKit.call("MakeThreadRealtime", thread, priority);
        if (reply.type() == QDBusMessage::ErrorMessage )
            qWarning() << "error returned by RealtimeKit.MakeThreadRealtime:"
                        << reply.errorMessage();
#else
        qWarning() << "pthread_setschedparam() failed, err="
                   << rt << ::strerror(rt);
#endif
    }
}

/**
 * Main input thread process loop.
 */
void
MidiClient::SequencerInputThread::run()
{
    unsigned long npfd;
    pollfd* pfd;
    if ( priority() == TimeCriticalPriority )
        setRealtimePriority();

    if (m_MidiClient != NULL) {
        npfd = snd_seq_poll_descriptors_count(m_MidiClient->getHandle(), POLLIN);
        pfd = (pollfd *) alloca(npfd * sizeof(pollfd));
        try
        {
            snd_seq_poll_descriptors(m_MidiClient->getHandle(), pfd, npfd, POLLIN);
            while (!stopped() && (m_MidiClient != NULL))
            {
                int rt = poll(pfd, npfd, m_Wait);
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

/**
 * Default constructor
 */
ClientInfo::ClientInfo()
{
    snd_seq_client_info_malloc(&m_Info);
}

/**
 * Copy constructor
 * @param other Another ClientInfo reference to be copied
 */
ClientInfo::ClientInfo(const ClientInfo& other)
{
    snd_seq_client_info_malloc(&m_Info);
    snd_seq_client_info_copy(m_Info, other.m_Info);
    m_Ports = other.m_Ports;
}

/**
 * Copy constructor
 * @param other An existing ALSA client info object
 */
ClientInfo::ClientInfo(snd_seq_client_info_t* other)
{
    snd_seq_client_info_malloc(&m_Info);
    snd_seq_client_info_copy(m_Info, other);
}

/**
 * Constructor
 * @param seq A MidiClient object
 * @param id A numeric client id
 */
ClientInfo::ClientInfo(MidiClient* seq, int id)
{
    snd_seq_client_info_malloc(&m_Info);
    snd_seq_get_any_client_info(seq->getHandle(), id, m_Info);
}

/**
 * Destructor
 */
ClientInfo::~ClientInfo()
{
    freePorts();
    snd_seq_client_info_free(m_Info);
}

/**
 * Clone the client info object.
 * @return A pointer to the new object.
 */
ClientInfo*
ClientInfo::clone()
{
    return new ClientInfo(m_Info);
}

/**
 * Assignment operator
 * @param other Another ClientInfo object
 * @return This object
 */
ClientInfo&
ClientInfo::operator=(const ClientInfo& other)
{
    snd_seq_client_info_copy(m_Info, other.m_Info);
    m_Ports = other.m_Ports;
    return *this;
}

/**
 * Gets the client's numeric identifier.
 * @return The client's numeric identifier.
 */
int
ClientInfo::getClientId()
{
    return snd_seq_client_info_get_client(m_Info);
}

/**
 * Gets the client's type
 * @return The client's type.
 */
snd_seq_client_type_t
ClientInfo::getClientType()
{
    return snd_seq_client_info_get_type(m_Info);
}

/**
 * Gets the client's name
 * @return The client's name.
 */
QString
ClientInfo::getName()
{
    return QString(snd_seq_client_info_get_name(m_Info));
}

/**
 * Gets the client's broadcast filter
 * @return The client's broadcast filter.
 */
bool
ClientInfo::getBroadcastFilter()
{
    return (snd_seq_client_info_get_broadcast_filter(m_Info) != 0);
}

/**
 * Gets the client's error bounce
 * @return The client's error bounce.
 */
bool
ClientInfo::getErrorBounce()
{
    return (snd_seq_client_info_get_error_bounce(m_Info) != 0);
}

/**
 * Gets the client's event filter.
 * @return The client's event filter.
 * @deprecated
 */
const unsigned char*
ClientInfo::getEventFilter()
{
    return snd_seq_client_info_get_event_filter(m_Info);
}

/**
 * Gets the client's port count.
 * @return The client's port count.
 */
int
ClientInfo::getNumPorts()
{
    return snd_seq_client_info_get_num_ports(m_Info);
}

/**
 * Gets the number of lost events.
 * @return The number of lost events.
 */
int
ClientInfo::getEventLost()
{
    return snd_seq_client_info_get_event_lost(m_Info);
}

/**
 * Sets the client identifier number.
 * @param client The client identifier number.
 */
void
ClientInfo::setClient(int client)
{
    snd_seq_client_info_set_client(m_Info, client);
}

/**
 * Sets the client name.
 * @param name The client name.
 */
void
ClientInfo::setName(QString name)
{
    snd_seq_client_info_set_name(m_Info, name.toLocal8Bit().data());
}

/**
 * Sets the broadcast filter.
 * @param val The broadcast filter.
 */
void
ClientInfo::setBroadcastFilter(bool val)
{
    snd_seq_client_info_set_broadcast_filter(m_Info, val ? 1 : 0);
}

/**
 * Sets the error bounce.
 * @param val The error bounce.
 */
void
ClientInfo::setErrorBounce(bool val)
{
    snd_seq_client_info_set_error_bounce(m_Info, val ? 1 : 0);
}

/**
 * Sets the event filter.
 * @param filter The event filter.
 * @deprecated
 */
void
ClientInfo::setEventFilter(unsigned char *filter)
{
    snd_seq_client_info_set_event_filter(m_Info, filter);
}

/**
 * Read the client ports.
 * @param seq The client instance.
 */
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

/**
 * Release the ports list.
 */
void
ClientInfo::freePorts()
{
    m_Ports.clear();
}

/**
 * Gets the ports list.
 * @return The ports list.
 */
PortInfoList
ClientInfo::getPorts() const
{
    PortInfoList lst = m_Ports; // copy
    return lst;
}

/**
 * Gets the size of the internal object.
 * @return The size of the internal object.
 */
int
ClientInfo::getSizeOfInfo() const
{
    return snd_seq_client_info_sizeof();
}

#if SND_LIB_VERSION > 0x010010
/**
 * Adds an event type to the client's filter.
 *
 * @param eventType The new event's type.
 */
void
ClientInfo::addFilter(int eventType)
{
    snd_seq_client_info_event_filter_add(m_Info, eventType);
}

/**
 * Checks id the given event's type is filtered.
 * @param eventType The event's type.
 * @return true if the event type is filtered
 */
bool
ClientInfo::isFiltered(int eventType)
{
    return (snd_seq_client_info_event_filter_check(m_Info, eventType) != 0);
}

/**
 * Clear the client's event filter
 */
void
ClientInfo::clearFilter()
{
    snd_seq_client_info_event_filter_clear(m_Info);
}

/**
 * Removes the event type from the client's filter.
 * @param eventType The event's type.
 */
void
ClientInfo::removeFilter(int eventType)
{
    snd_seq_client_info_event_filter_del(m_Info, eventType);
}
#endif

/**
 * Default constructor
 */
SystemInfo::SystemInfo()
{
    snd_seq_system_info_malloc(&m_Info);
}

/**
 * Copy constructor
 * @param other Another SystemInfo object reference to be copied
 */
SystemInfo::SystemInfo(const SystemInfo& other)
{
    snd_seq_system_info_malloc(&m_Info);
    snd_seq_system_info_copy(m_Info, other.m_Info);
}

/**
 * Copy constructor
 * @param other Another ALSA system info object to be copied
 */
SystemInfo::SystemInfo(snd_seq_system_info_t* other)
{
    snd_seq_system_info_malloc(&m_Info);
    snd_seq_system_info_copy(m_Info, other);
}

/**
 * Constructor
 * @param seq A MidiClient object
 */
SystemInfo::SystemInfo(MidiClient* seq)
{
    snd_seq_system_info_malloc(&m_Info);
    snd_seq_system_info(seq->getHandle(), m_Info);
}

/**
 * Destructor
 */
SystemInfo::~SystemInfo()
{
    snd_seq_system_info_free(m_Info);
}

/**
 * Clone the system info object
 * @return A pointer to the new object
 */
SystemInfo*
SystemInfo::clone()
{
    return new SystemInfo(m_Info);
}

/**
 * Assignment operator
 * @param other Another SystemInfo object
 * @return This object
 */
SystemInfo&
SystemInfo::operator=(const SystemInfo& other)
{
    snd_seq_system_info_copy(m_Info, other.m_Info);
    return *this;
}

/**
 * Get the system's maximum number of clients.
 * @return The maximum number of clients.
 */
int SystemInfo::getMaxClients()
{
    return snd_seq_system_info_get_clients(m_Info);
}

/**
 * Get the system's maximum number of ports.
 * @return The maximum number of ports.
 */
int SystemInfo::getMaxPorts()
{
    return snd_seq_system_info_get_ports(m_Info);
}

/**
 * Get the system's maximum number of queues.
 * @return The system's maximum number of queues.
 */
int SystemInfo::getMaxQueues()
{
    return snd_seq_system_info_get_queues(m_Info);
}

/**
 * Get the system's maximum number of channels.
 * @return The system's maximum number of channels.
 */
int SystemInfo::getMaxChannels()
{
    return snd_seq_system_info_get_channels(m_Info);
}

/**
 * Get the system's current number of queues.
 * @return The system's current number of queues.
 */
int SystemInfo::getCurrentQueues()
{
    return snd_seq_system_info_get_cur_queues(m_Info);
}

/**
 * Get the system's current number of clients.
 * @return The system's current number of clients.
 */
int SystemInfo::getCurrentClients()
{
    return snd_seq_system_info_get_cur_clients(m_Info);
}

/**
 * Get the system's info object size.
 * @return The system's info object size.
 */
int SystemInfo::getSizeOfInfo() const
{
    return snd_seq_system_info_sizeof();
}

/**
 * Default constructor
 */
PoolInfo::PoolInfo()
{
    snd_seq_client_pool_malloc(&m_Info);
}

/**
 * Copy constructor
 * @param other Another PoolInfo object reference to be copied
 */
PoolInfo::PoolInfo(const PoolInfo& other)
{
    snd_seq_client_pool_malloc(&m_Info);
    snd_seq_client_pool_copy(m_Info, other.m_Info);
}

/**
 * Copy constructor
 * @param other An ALSA pool info object to be copied
 */
PoolInfo::PoolInfo(snd_seq_client_pool_t* other)
{
    snd_seq_client_pool_malloc(&m_Info);
    snd_seq_client_pool_copy(m_Info, other);
}

/**
 * Constructor
 * @param seq A MidiClient object
 */
PoolInfo::PoolInfo(MidiClient* seq)
{
    snd_seq_client_pool_malloc(&m_Info);
    snd_seq_get_client_pool(seq->getHandle(), m_Info);
}

/**
 * Destructor
 */
PoolInfo::~PoolInfo()
{
    snd_seq_client_pool_free(m_Info);
}

/**
 * Clone the pool info obeject
 * @return A pointer to the new object
 */
PoolInfo*
PoolInfo::clone()
{
    return new PoolInfo(m_Info);
}

/**
 * Assignment operator
 * @param other Another PoolInfo object reference to be copied
 * @return This object
 */
PoolInfo& PoolInfo::operator=(const PoolInfo& other)
{
    snd_seq_client_pool_copy(m_Info, other.m_Info);
    return *this;
}

/**
 * Gets the client ID for this object.
 * @return The client ID.
 */
int
PoolInfo::getClientId()
{
    return snd_seq_client_pool_get_client(m_Info);
}

/**
 * Gets the available size on input pool.
 * @return The available size on input pool.
 */
int
PoolInfo::getInputFree()
{
    return snd_seq_client_pool_get_input_free(m_Info);
}

/**
 * Gets the input pool size.
 * @return The input pool size.
 */
int
PoolInfo::getInputPool()
{
    return snd_seq_client_pool_get_input_pool(m_Info);
}

/**
 * Gets the available size on output pool.
 * @return The available size on output pool.
 */
int
PoolInfo::getOutputFree()
{
    return snd_seq_client_pool_get_output_free(m_Info);
}

/**
 * Gets the output pool size.
 * @return The output pool size.
 */
int
PoolInfo::getOutputPool()
{
    return snd_seq_client_pool_get_output_pool(m_Info);
}

/**
 * Gets the output room size.
 * The output room is the minimum pool size for select/blocking mode.
 * @return The output room size.
 */
int
PoolInfo::getOutputRoom()
{
    return snd_seq_client_pool_get_output_room(m_Info);
}

/**
 * Set the input pool size.
 * @param size The input pool size.
 */
void
PoolInfo::setInputPool(int size)
{
    snd_seq_client_pool_set_input_pool(m_Info, size);
}

/**
 * Sets the output pool size.
 * @param size The output pool size.
 */
void
PoolInfo::setOutputPool(int size)
{
    snd_seq_client_pool_set_output_pool(m_Info, size);
}

/**
 * Sets the output room size.
 * The output room is the minimum pool size for select/blocking mode.
 *
 * @param size Output room size
 */
void
PoolInfo::setOutputRoom(int size)
{
    snd_seq_client_pool_set_output_room(m_Info, size);
}

/**
 * Gets the size of the client pool object.
 * @return The size of the client pool object.
 */
int
PoolInfo::getSizeOfInfo() const
{
    return snd_seq_client_pool_sizeof();
}

#if SND_LIB_VERSION > 0x010004
/**
 * Gets the runtime ALSA library version string
 * @return string representing the runtime ALSA library version
 * @since 0.3.0
 */
QString
getRuntimeALSALibraryVersion()
{
    return QString(snd_asoundlib_version());
}

/**
 * Gets the runtime ALSA library version number
 * @return integer representing the runtime ALSA library version
 * @since 0.3.0
 */
int
getRuntimeALSALibraryNumber()
{
    QRegExp rx("(\\d+)");
    QString str = getRuntimeALSALibraryVersion();
    bool ok;
    int pos = 0, result = 0, j = 0;
    while ((pos = rx.indexIn(str, pos)) != -1 && j < 3) {
        int v = rx.cap(1).toInt(&ok);
        if (ok) {
            result <<= 8;
            result += v;
        }
        pos += rx.matchedLength();
        j++;
    }
    return result;
}
#endif // SND_LIB_VERSION > 0x010004

/**
 * Gets the runtime ALSA drivers version string
 * @return string representing the runtime ALSA drivers version
 * @since 0.3.0
 */
QString
getRuntimeALSADriverVersion()
{
    QRegExp rx(".*Driver Version.*([\\d\\.]+).*");
    QString s;
    QFile f("/proc/asound/version");
    if (f.open(QFile::ReadOnly)) {
        QTextStream str(&f);
        if (rx.exactMatch(str.readLine().trimmed()))
            s = rx.cap(1);
    }
    return s;
}

/**
 * Gets the runtime ALSA drivers version number
 * @return integer representing the runtime ALSA drivers version
 * @since 0.3.0
 */
int
getRuntimeALSADriverNumber()
{
    QRegExp rx("(\\d+)");
    QString str = getRuntimeALSADriverVersion();
    bool ok;
    int pos = 0, result = 0, j = 0;
    while ((pos = rx.indexIn(str, pos)) != -1 && j < 3) {
        int v = rx.cap(1).toInt(&ok);
        if (ok) {
            result <<= 8;
            result += v;
        }
        pos += rx.matchedLength();
        j++;
    }
    return result;
}

} /* namespace drumstick */
