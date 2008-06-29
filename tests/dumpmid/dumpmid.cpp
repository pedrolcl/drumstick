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

#include <signal.h>
#include <iostream>
#include <iomanip> 

#include <QObject>
#include <QString>
#include <QApplication>

#include "dumpmid.h"

using namespace std;
using namespace ALSA::Sequencer;

QDumpMIDI::QDumpMIDI()
	: QObject()
{
	m_Client = new MidiClient(this);
	m_Port = new MidiPort(this);
	m_Client->setOpenMode(SND_SEQ_OPEN_DUPLEX);
	m_Client->setBlockMode(false);
	m_Client->open();
	m_Client->setClientName("DumpMIDI");

	m_Port->setMidiClient(m_Client);
	m_Port->setPortName("DumpMIDI port");
	m_Port->setCapability(SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE);
	m_Port->setPortType(SND_SEQ_PORT_TYPE_APPLICATION);
	connect(m_Port, SIGNAL(subscribed(MidiPort*,Subscription*)), SLOT(subscription(MidiPort*,Subscription*)));
	
	m_Port->attach();
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
	m.lock();
	bool bTmp = m_Stopped;
	m.unlock();
	return bTmp;
}

void
QDumpMIDI::stop()
{
	m.lock();
	m_Stopped = true;
	m.unlock();
}

void 
QDumpMIDI::subscription(MidiPort*, Subscription* subs)
{
	qDebug("Subscription made from %d:%d", subs->getSender()->client, subs->getSender()->port);
	subs->setQueue(m_Client->getQueue()->getId());
	subs->setTimeReal(true);
	subs->setTimeUpdate(true);
}

void QDumpMIDI::subscribe(const QString& portName)
{
	try	{
		qDebug("Trying to subscribe %s", portName.toLocal8Bit().data());
		m_Port->subscribeFrom(portName);
	} catch (FatalError *err) {
		cerr << "FatalError exception. Error code: " << err->code() 
			 << " (" << err->qstrError().toStdString() << ")" << endl;
		cerr << "Location: " << err->what() << endl;
		throw err;
	}
}

void QDumpMIDI::run()
{
	cout << "Press Ctrl+C to exit" << endl;
	cout << "Source_ Event_________________ Ch _Data__" << endl;
	try	{
		m_Client->createQueue();
		m_Client->startEvents();
		m_Client->getQueue()->start();
		m_Stopped = false;
		while (!stopped()) {
			sleep(1);
		}
		m_Client->getQueue()->stop();
		m_Client->stopEvents();
	} catch (FatalError *err) {
		cerr << "FatalError exception. Error code: " << err->code() 
			 << " (" << err->qstrError().toStdString() << ")" << endl;
		cerr << "Location: " << err->what() << endl;
		throw err;
	}
}

void 
QDumpMIDI::customEvent(QEvent *ev)
{
	if (ev->type() == SequencerEventType) {
		SequencerEvent* sev = dynamic_cast<SequencerEvent*>(ev);
		if (sev != NULL) {
			dumpEvent(sev);
		}
	}
	delete ev;
}

void 
QDumpMIDI::dumpEvent(SequencerEvent* sev)
{
	cout << setw(3) << right << sev->getSourceClient() << ":";
	cout << setw(3) << left << sev->getSourcePort() << " ";
	
	switch (sev->getType()) {
	case SND_SEQ_EVENT_NOTEON: {
		NoteOnEvent* e = dynamic_cast<NoteOnEvent*>(sev);
		cout << setw(23) << left << "Note on";
		cout << setw(2) << right << e->getChannel() << " ";
		cout << setw(3) << e->getKey() << " ";
		cout << setw(3) << e->getVelocity();
		break;
	}
	case SND_SEQ_EVENT_NOTEOFF: {
		NoteOffEvent* e = dynamic_cast<NoteOffEvent*>(sev);
		cout << setw(23) << left << "Note off";
		cout << setw(2) << right << e->getChannel() << " ";
		cout << setw(3) << e->getKey() << " ";
		cout << setw(3) << e->getVelocity();
		break;
	}
	case SND_SEQ_EVENT_KEYPRESS: {
		KeyPressEvent* e = dynamic_cast<KeyPressEvent*>(sev);
		cout << setw(23) << left << "Polyphonic aftertouch";
		cout << setw(2) << right << e->getChannel() << " ";
		cout << setw(3) << e->getKey() << " ";
		cout << setw(3) << e->getVelocity();
		break;
	}
	case SND_SEQ_EVENT_CONTROL14:
	case SND_SEQ_EVENT_NONREGPARAM:
	case SND_SEQ_EVENT_REGPARAM:
	case SND_SEQ_EVENT_CONTROLLER: {
		ControllerEvent* e = dynamic_cast<ControllerEvent*>(sev);
		cout << setw(23) << left << "Control change";
		cout << setw(2) << right << e->getChannel() << " ";
		cout << setw(3) << e->getParam() << " ";
		cout << setw(3) << e->getValue();
		break;
	}
	case SND_SEQ_EVENT_PGMCHANGE: {
		ProgramChangeEvent* e = dynamic_cast<ProgramChangeEvent*>(sev);
		cout << setw(23) << left << "Program change";
		cout << setw(2) << right << e->getChannel() << " ";
		cout << setw(3) << e->getValue();
		break;
	}
	case SND_SEQ_EVENT_CHANPRESS: {
		ChanPressEvent* e = dynamic_cast<ChanPressEvent*>(sev);
		cout << setw(23) << left << "Channel aftertouch";
		cout << setw(2) << right << e->getChannel() << " ";
		cout << setw(3) << e->getValue();
		break;
	}
	case SND_SEQ_EVENT_PITCHBEND: {
		PitchBendEvent* e = dynamic_cast<PitchBendEvent*>(sev);
		cout << setw(23) << left << "Pitch bend";
		cout << setw(2) << right << e->getChannel() << " ";
		cout << setw(5) << e->getValue();
		break;
	}
	case SND_SEQ_EVENT_SONGPOS: {
		ValueEvent* e = dynamic_cast<ValueEvent*>(sev);
		cout << setw(26) << left << "Song position pointer";
		cout << e->getValue();
		break;
	}
	case SND_SEQ_EVENT_SONGSEL: {
		ValueEvent* e = dynamic_cast<ValueEvent*>(sev);
		cout << setw(26) << left << "Song select";
		cout << e->getValue();
		break;
	}
	case SND_SEQ_EVENT_QFRAME: {
		ValueEvent* e = dynamic_cast<ValueEvent*>(sev);
		cout << setw(26) << left << "MTC quarter frame";
		cout << e->getValue();
		break;
	}
	case SND_SEQ_EVENT_TIMESIGN: {
		ValueEvent* e = dynamic_cast<ValueEvent*>(sev);
		cout << setw(26) << left << "SMF time signature";
		cout << hex << e->getValue();
		cout << dec;
		break;
	}
	case SND_SEQ_EVENT_KEYSIGN: {
		ValueEvent* e = dynamic_cast<ValueEvent*>(sev);
		cout << setw(26) << left << "SMF key signature";
		cout << hex << e->getValue();
		cout << dec;
		break;
	}
	case SND_SEQ_EVENT_SETPOS_TICK: {
		QueueControlEvent* e = dynamic_cast<QueueControlEvent*>(sev);
		cout << setw(26) << left << "Set tick queue pos.";
		cout << e->getQueue();
		break;
	}
	case SND_SEQ_EVENT_SETPOS_TIME: {
		QueueControlEvent* e = dynamic_cast<QueueControlEvent*>(sev);
		cout << setw(26) << left << "Set rt queue pos.";
		cout << e->getQueue();
		break;
	}
	case SND_SEQ_EVENT_TEMPO: {
		TempoEvent* e = dynamic_cast<TempoEvent*>(sev);
		cout << setw(26) << left << "Set queue tempo";
		cout << setw(3) << right << e->getQueue() << " ";
		cout << e->getValue();
		break;
	}
	case SND_SEQ_EVENT_QUEUE_SKEW: {
		QueueControlEvent* e = dynamic_cast<QueueControlEvent*>(sev);
		cout << setw(26) << left << "Queue timer skew";
		cout << e->getQueue();
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
		ClientEvent* e = dynamic_cast<ClientEvent*>(sev); 
		cout << setw(26) << left << "Client start";
		cout << e->getClient();
		break;
	}
	case SND_SEQ_EVENT_CLIENT_EXIT: {
		ClientEvent* e = dynamic_cast<ClientEvent*>(sev); 
		cout << setw(26) << left << "Client exit";
		cout << e->getClient();
		break;
	}
	case SND_SEQ_EVENT_CLIENT_CHANGE: {
		ClientEvent* e = dynamic_cast<ClientEvent*>(sev); 
		cout << setw(26) << left << "Client changed";
		cout << e->getClient();
		break;
	}
	case SND_SEQ_EVENT_PORT_START: {
		PortEvent* e = dynamic_cast<PortEvent*>(sev); 
		cout << setw(26) << left << "Port start";
		cout << e->getClient() << ":" << e->getPort();
		break;
	}
	case SND_SEQ_EVENT_PORT_EXIT: {
		PortEvent* e = dynamic_cast<PortEvent*>(sev); 
		cout << setw(26) << left << "Port exit";
		cout << e->getClient() << ":" << e->getPort();
		break;
	}
	case SND_SEQ_EVENT_PORT_CHANGE: {
		PortEvent* e = dynamic_cast<PortEvent*>(sev); 
		cout << setw(26) << left << "Port changed";
		cout << e->getClient() << ":" << e->getPort();
		break;
	}
	case SND_SEQ_EVENT_PORT_SUBSCRIBED: {
		SubscriptionEvent* e = dynamic_cast<SubscriptionEvent*>(sev);
		cout << setw(26) << left << "Port subscribed";
		cout << e->getSenderClient() << ":" << e->getSenderPort() << " -> ";
		cout << e->getDestClient() << ":" << e->getDestPort();
		break;
	}
	case SND_SEQ_EVENT_PORT_UNSUBSCRIBED: {
		SubscriptionEvent* e = dynamic_cast<SubscriptionEvent*>(sev);
		cout << setw(26) << left << "Port unsubscribed";
		cout << e->getSenderClient() << ":" << e->getSenderPort() << " -> ";
		cout << e->getDestClient() << ":" << e->getDestPort();
		break;
	}
	case SND_SEQ_EVENT_SYSEX: {
		SysExEvent* e = dynamic_cast<SysExEvent*>(sev);
		cout << setw(26) << left << "System exclusive";
		unsigned int i;
		for (i = 0; i < e->getLength(); ++i) {
			cout << hex << (int) e->getData()[i] << " ";
		}
		cout << dec;
		break;
	}
	default:
		cout << setw(26) << "Unknown event type";
		cout << sev->getType();
	};
	cout << endl;
}

QDumpMIDI* test;

void signalHandler(int sig)
{
	if (sig == SIGINT)
		qDebug("Caught a SIGINT. Exiting");
	else if (sig == SIGTERM)
		qDebug("Caught a SIGTERM. Exiting");
	test->stop();
}

int main(int argc, char **argv)
{
	QApplication app(argc, argv, false);
	test = new QDumpMIDI();
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	if (argc > 1)
	{
		QString portName(argv[1]);
		test->subscribe(portName);
	}
	test->run();
	delete test;
	return 0;
}
