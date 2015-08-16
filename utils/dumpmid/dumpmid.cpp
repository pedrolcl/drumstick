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

#include "dumpmid.h"
#include "cmdlineargs.h"

#include <signal.h>
#include <QObject>
#include <QString>
#include <QCoreApplication>
#include <QTextStream>
#include <QtDebug>
#include <QReadLocker>
#include <QWriteLocker>

static QTextStream cout(stdout, QIODevice::WriteOnly);
static QTextStream cerr(stderr, QIODevice::WriteOnly);

QDumpMIDI::QDumpMIDI()
    : QObject()
{
    m_Client = new MidiClient(this);
    m_Client->open();
    m_Client->setClientName("DumpMIDI");
#ifndef USE_QEVENTS // using signals instead
    connect( m_Client, SIGNAL(eventReceived(SequencerEvent*)),
                       SLOT(sequencerEvent(SequencerEvent*)),
                       Qt::DirectConnection );
#endif
    m_Port = new MidiPort(this);
    m_Port->attach( m_Client );
    m_Port->setPortName("DumpMIDI port");
    m_Port->setCapability( SND_SEQ_PORT_CAP_WRITE |
                           SND_SEQ_PORT_CAP_SUBS_WRITE );
    m_Port->setPortType( SND_SEQ_PORT_TYPE_APPLICATION |
                         SND_SEQ_PORT_TYPE_MIDI_GENERIC );
#ifdef WANT_TIMESTAMPS
    m_Queue = m_Client->createQueue("DumpMIDI");
    m_Port->setTimestamping(true);
    //m_Port->setTimestampReal(true);
    m_Port->setTimestampQueue(m_Queue->getId());
#endif
    connect( m_Port, SIGNAL(subscribed(MidiPort*,Subscription*)),
                     SLOT(subscription(MidiPort*,Subscription*)));
    qDebug() << "Trying to subscribe from Announce";
    m_Port->subscribeFromAnnounce();
}

QDumpMIDI::~QDumpMIDI()
{
    m_Port->detach();
    delete m_Port;
    m_Client->close();
    delete m_Client;
}

bool
QDumpMIDI::stopped()
{
	QReadLocker locker(&m_mutex);
    return m_Stopped;
}

void
QDumpMIDI::stop()
{
	QWriteLocker locker(&m_mutex);
	m_Stopped = true;
}

void
QDumpMIDI::subscription(MidiPort*, Subscription* subs)
{
    qDebug() << "Subscription made from"
             << subs->getSender()->client << ":"
             << subs->getSender()->port;
}

void QDumpMIDI::subscribe(const QString& portName)
{
    try {
        qDebug() << "Trying to subscribe" << portName.toLocal8Bit().data();
        m_Port->subscribeFrom(portName);
    } catch (const SequencerError& err) {
        cerr << "SequencerError exception. Error code: " << err.code()
             << " (" << err.qstrError() << ")" << endl;
        cerr << "Location: " << err.location() << endl;
        throw err;
    }
}

void QDumpMIDI::run()
{
    cout << "Press Ctrl+C to exit" << endl;
#ifdef WANT_TIMESTAMPS
    cout << "___Ticks ";
#endif
    cout << "Source_ Event_________________ Ch _Data__" << endl;
    try {
#ifdef USE_QEVENTS
        m_Client->addListener(this);
        m_Client->setEventsEnabled(true);
#endif
        m_Client->setRealTimeInput(false);
        m_Client->startSequencerInput();
#ifdef WANT_TIMESTAMPS
        m_Queue->start();
#endif
        m_Stopped = false;
        while (!stopped()) {
#ifdef USE_QEVENTS
            QApplication::sendPostedEvents();
#endif
            sleep(1);
        }
#ifdef WANT_TIMESTAMPS
        m_Queue->stop();
#endif
        m_Client->stopSequencerInput();
    } catch (const SequencerError& err) {
        cerr << "SequencerError exception. Error code: " << err.code()
             << " (" << err.qstrError() << ")" << endl;
        cerr << "Location: " << err.location() << endl;
        throw err;
    }
}

#ifdef USE_QEVENTS
void
QDumpMIDI::customEvent(QEvent *ev)
{
    if (ev->type() == SequencerEventType) {
        SequencerEvent* sev = static_cast<SequencerEvent*>(ev);
        if (sev != NULL) {
            dumpEvent(sev);
        }
    }
}
#else
void
QDumpMIDI::sequencerEvent(SequencerEvent *ev)
{
    dumpEvent(ev);
    delete ev;
}
#endif

void
QDumpMIDI::dumpEvent(SequencerEvent* sev)
{
#ifdef WANT_TIMESTAMPS
    cout << qSetFieldWidth(8) << right << sev->getTick();
    /* More timestamp options:
    cout << sev->getRealTimeSecs();
    cout << sev->getRealTimeNanos(); */
    /* Getting the time from the queue status object;
    QueueStatus sts = m_Queue->getStatus();
    cout << qSetFieldWidth(8) << right << sts.getClockTime();
    cout << sts.getTickTime(); */
    cout << qSetFieldWidth(0) << " ";
#endif

    cout << qSetFieldWidth(3) << right << sev->getSourceClient() << qSetFieldWidth(0) << ":";
    cout << qSetFieldWidth(3) << left << sev->getSourcePort() << qSetFieldWidth(0) << " ";

    switch (sev->getSequencerType()) {
    case SND_SEQ_EVENT_NOTEON: {
        NoteOnEvent* e = static_cast<NoteOnEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(23) << left << "Note on";
            cout << qSetFieldWidth(2) << right << e->getChannel() << " ";
            cout << qSetFieldWidth(3) << e->getKey() << " ";
            cout << qSetFieldWidth(3) << e->getVelocity();
        }
        break;
    }
    case SND_SEQ_EVENT_NOTEOFF: {
        NoteOffEvent* e = static_cast<NoteOffEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(23) << left << "Note off";
            cout << qSetFieldWidth(2) << right << e->getChannel() << " ";
            cout << qSetFieldWidth(3) << e->getKey() << " ";
            cout << qSetFieldWidth(3) << e->getVelocity();
        }
        break;
    }
    case SND_SEQ_EVENT_KEYPRESS: {
        KeyPressEvent* e = static_cast<KeyPressEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(23) << left << "Polyphonic aftertouch";
            cout << qSetFieldWidth(2) << right << e->getChannel() << " ";
            cout << qSetFieldWidth(3) << e->getKey() << " ";
            cout << qSetFieldWidth(3) << e->getVelocity();
        }
        break;
    }
    case SND_SEQ_EVENT_CONTROL14:
    case SND_SEQ_EVENT_NONREGPARAM:
    case SND_SEQ_EVENT_REGPARAM:
    case SND_SEQ_EVENT_CONTROLLER: {
        ControllerEvent* e = static_cast<ControllerEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(23) << left << "Control change";
            cout << qSetFieldWidth(2) << right << e->getChannel() << " ";
            cout << qSetFieldWidth(3) << e->getParam() << " ";
            cout << qSetFieldWidth(3) << e->getValue();
        }
        break;
    }
    case SND_SEQ_EVENT_PGMCHANGE: {
        ProgramChangeEvent* e = static_cast<ProgramChangeEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(23) << left << "Program change";
            cout << qSetFieldWidth(2) << right << e->getChannel() << " ";
            cout << qSetFieldWidth(3) << e->getValue();
        }
        break;
    }
    case SND_SEQ_EVENT_CHANPRESS: {
        ChanPressEvent* e = static_cast<ChanPressEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(23) << left << "Channel aftertouch";
            cout << qSetFieldWidth(2) << right << e->getChannel() << " ";
            cout << qSetFieldWidth(3) << e->getValue();
        }
        break;
    }
    case SND_SEQ_EVENT_PITCHBEND: {
        PitchBendEvent* e = static_cast<PitchBendEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(23) << left << "Pitch bend";
            cout << qSetFieldWidth(2) << right << e->getChannel() << " ";
            cout << qSetFieldWidth(5) << e->getValue();
        }
        break;
    }
    case SND_SEQ_EVENT_SONGPOS: {
        ValueEvent* e = static_cast<ValueEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(26) << left << "Song position pointer" << qSetFieldWidth(0);
            cout << e->getValue();
        }
        break;
    }
    case SND_SEQ_EVENT_SONGSEL: {
        ValueEvent* e = static_cast<ValueEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(26) << left << "Song select" << qSetFieldWidth(0);
            cout << e->getValue();
        }
        break;
    }
    case SND_SEQ_EVENT_QFRAME: {
        ValueEvent* e = static_cast<ValueEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(26) << left << "MTC quarter frame" << qSetFieldWidth(0);
            cout << e->getValue();
        }
        break;
    }
    case SND_SEQ_EVENT_TIMESIGN: {
        ValueEvent* e = static_cast<ValueEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(26) << left << "SMF time signature" << qSetFieldWidth(0);
            cout << hex << e->getValue();
            cout << dec;
        }
        break;
    }
    case SND_SEQ_EVENT_KEYSIGN: {
        ValueEvent* e = static_cast<ValueEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(26) << left << "SMF key signature" << qSetFieldWidth(0);
            cout << hex << e->getValue();
            cout << dec;
        }
        break;
    }
    case SND_SEQ_EVENT_SETPOS_TICK: {
        QueueControlEvent* e = static_cast<QueueControlEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(26) << left << "Set tick queue pos." << qSetFieldWidth(0);
            cout << e->getQueue();
        }
        break;
    }
    case SND_SEQ_EVENT_SETPOS_TIME: {
        QueueControlEvent* e = static_cast<QueueControlEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(26) << left << "Set rt queue pos." << qSetFieldWidth(0);
            cout << e->getQueue();
        }
        break;
    }
    case SND_SEQ_EVENT_TEMPO: {
        TempoEvent* e = static_cast<TempoEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(26) << left << "Set queue tempo";
            cout << qSetFieldWidth(3) << right << e->getQueue() << qSetFieldWidth(0) << " ";
            cout << e->getValue();
        }
        break;
    }
    case SND_SEQ_EVENT_QUEUE_SKEW: {
        QueueControlEvent* e = static_cast<QueueControlEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(26) << left << "Queue timer skew" << qSetFieldWidth(0);
            cout << e->getQueue();
        }
        break;
    }
    case SND_SEQ_EVENT_START:
        cout << left << "Start";
        break;
    case SND_SEQ_EVENT_STOP:
        cout << left << "Stop";
        break;
    case SND_SEQ_EVENT_CONTINUE:
        cout << left << "Continue";
        break;
    case SND_SEQ_EVENT_CLOCK:
        cout << left << "Clock";
        break;
    case SND_SEQ_EVENT_TICK:
        cout << left << "Tick";
        break;
    case SND_SEQ_EVENT_TUNE_REQUEST:
        cout << left << "Tune request";
        break;
    case SND_SEQ_EVENT_RESET:
        cout << left << "Reset";
        break;
    case SND_SEQ_EVENT_SENSING:
        cout << left << "Active Sensing";
        break;
    case SND_SEQ_EVENT_CLIENT_START: {
        ClientEvent* e = static_cast<ClientEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(26) << left << "Client start"
                 << qSetFieldWidth(0) << e->getClient();
        }
        break;
    }
    case SND_SEQ_EVENT_CLIENT_EXIT: {
        ClientEvent* e = static_cast<ClientEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(26) << left << "Client exit"
                 << qSetFieldWidth(0) << e->getClient();
        }
        break;
    }
    case SND_SEQ_EVENT_CLIENT_CHANGE: {
        ClientEvent* e = static_cast<ClientEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(26) << left << "Client changed"
                 << qSetFieldWidth(0) << e->getClient();
        }
        break;
    }
    case SND_SEQ_EVENT_PORT_START: {
        PortEvent* e = static_cast<PortEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(26) << left << "Port start" << qSetFieldWidth(0);
            cout << e->getClient() << ":" << e->getPort();
        }
        break;
    }
    case SND_SEQ_EVENT_PORT_EXIT: {
        PortEvent* e = static_cast<PortEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(26) << left << "Port exit" << qSetFieldWidth(0);
            cout << e->getClient() << ":" << e->getPort();
        }
        break;
    }
    case SND_SEQ_EVENT_PORT_CHANGE: {
        PortEvent* e = static_cast<PortEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(26) << left << "Port changed" << qSetFieldWidth(0);
            cout << e->getClient() << ":" << e->getPort();
        }
        break;
    }
    case SND_SEQ_EVENT_PORT_SUBSCRIBED: {
        SubscriptionEvent* e = static_cast<SubscriptionEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(26) << left << "Port subscribed" << qSetFieldWidth(0);
            cout << e->getSenderClient() << ":" << e->getSenderPort() << " -> ";
            cout << e->getDestClient() << ":" << e->getDestPort();
        }
        break;
    }
    case SND_SEQ_EVENT_PORT_UNSUBSCRIBED: {
        SubscriptionEvent* e = static_cast<SubscriptionEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(26) << left << "Port unsubscribed" << qSetFieldWidth(0);
            cout << e->getSenderClient() << ":" << e->getSenderPort() << " -> ";
            cout << e->getDestClient() << ":" << e->getDestPort();
        }
        break;
    }
    case SND_SEQ_EVENT_SYSEX: {
        SysExEvent* e = static_cast<SysExEvent*>(sev);
        if (e != NULL) {
            cout << qSetFieldWidth(26) << left << "System exclusive" << qSetFieldWidth(0);
            unsigned int i;
            for (i = 0; i < e->getLength(); ++i) {
                cout << hex << (unsigned char) e->getData()[i] << " ";
            }
            cout << dec;
        }
        break;
    }
    default:
        cout << qSetFieldWidth(26) << "Unknown event type" << qSetFieldWidth(0);
        cout << sev->getSequencerType();
    };
    cout << qSetFieldWidth(0) << endl;
}

QDumpMIDI* test;

void signalHandler(int sig)
{
    if (sig == SIGINT)
        qDebug() << "Caught a SIGINT. Exiting";
    else if (sig == SIGTERM)
        qDebug() << "Caught a SIGTERM. Exiting";
    test->stop();
}

int main(int argc, char **argv)
{
    const QString errorstr = "Fatal error from the ALSA sequencer. "
        "This usually happens when the kernel doesn't have ALSA support, "
        "or the device node (/dev/snd/seq) doesn't exists, "
        "or the kernel module (snd_seq) is not loaded. "
        "Please check your ALSA/MIDI configuration.";

    CmdLineArgs args;
    args.setUsage("[port]");
    args.addOptionalArgument("port", "Source MIDI port");
    args.parse(argc, argv);
    try {
        test = new QDumpMIDI();
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        QVariant portName = args.getArgument("port");
        if (!portName.isNull())
            test->subscribe(portName.toString());
        test->run();
    } catch (const SequencerError& ex) {
        cerr << errorstr + " Returned error was: " + ex.qstrError() << endl;
    } catch (...) {
        cerr << errorstr << endl;
    }
    delete test;
    return 0;
}
