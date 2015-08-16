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

#include "testevents.h"
#include "cmdlineargs.h"
#include <QObject>
#include <QString>
#include <QCoreApplication>
#include <QTextStream>

static QTextStream cout(stdout, QIODevice::WriteOnly);

void QTestEvents::dumpEvent(SequencerEvent* sev)
{
    cout << qSetFieldWidth(3) << right << sev->getSourceClient() << qSetFieldWidth(0) << ":";
    cout << qSetFieldWidth(3) << left << sev->getSourcePort() << qSetFieldWidth(0) << " ";

    switch (sev->getSequencerType()) {
    case SND_SEQ_EVENT_NOTE: {
        NoteEvent* e = static_cast<NoteEvent*>(sev);
        cout << qSetFieldWidth(23) << left << "Note";
        cout << qSetFieldWidth(2) << right << e->getChannel() << qSetFieldWidth(0) << " ";
        cout << qSetFieldWidth(3) << e->getKey() << qSetFieldWidth(0) << " ";
        cout << qSetFieldWidth(3) << e->getVelocity() << qSetFieldWidth(0) << " ";
        cout << e->getDuration();
        break;
    }
    case SND_SEQ_EVENT_NOTEON: {
        NoteOnEvent* e = static_cast<NoteOnEvent*>(sev);
        cout << qSetFieldWidth(23) << left << "Note on";
        cout << qSetFieldWidth(2) << right << e->getChannel() << qSetFieldWidth(0) << " ";
        cout << qSetFieldWidth(3) << e->getKey() << qSetFieldWidth(0) << " ";
        cout << qSetFieldWidth(3) << e->getVelocity();
        break;
    }
    case SND_SEQ_EVENT_NOTEOFF: {
        NoteOffEvent* e = static_cast<NoteOffEvent*>(sev);
        cout << qSetFieldWidth(23) << left << "Note off";
        cout << qSetFieldWidth(2) << right << e->getChannel() << qSetFieldWidth(0) << " ";
        cout << qSetFieldWidth(3) << e->getKey() << qSetFieldWidth(0) << " ";
        cout << qSetFieldWidth(3) << e->getVelocity();
        break;
    }
    case SND_SEQ_EVENT_KEYPRESS: {
        KeyPressEvent* e = static_cast<KeyPressEvent*>(sev);
        cout << qSetFieldWidth(23) << left << "Polyphonic aftertouch";
        cout << qSetFieldWidth(2) << right << e->getChannel() << qSetFieldWidth(0) << " ";
        cout << qSetFieldWidth(3) << e->getKey() << qSetFieldWidth(0) << " ";
        cout << qSetFieldWidth(3) << e->getVelocity();
        break;
    }
    case SND_SEQ_EVENT_CONTROL14:
    case SND_SEQ_EVENT_NONREGPARAM:
    case SND_SEQ_EVENT_REGPARAM:
    case SND_SEQ_EVENT_CONTROLLER: {
        ControllerEvent* e = static_cast<ControllerEvent*>(sev);
        cout << qSetFieldWidth(23) << left << "Control change";
        cout << qSetFieldWidth(2) << right << e->getChannel() << qSetFieldWidth(0) << " ";
        cout << qSetFieldWidth(3) << e->getParam() << qSetFieldWidth(0) << " ";
        cout << qSetFieldWidth(3) << e->getValue();
        break;
    }
    case SND_SEQ_EVENT_PGMCHANGE: {
        ProgramChangeEvent* e = static_cast<ProgramChangeEvent*>(sev);
        cout << qSetFieldWidth(23) << left << "Program change";
        cout << qSetFieldWidth(2) << right << e->getChannel() << qSetFieldWidth(0) << " ";
        cout << qSetFieldWidth(3) << e->getValue();
        break;
    }
    case SND_SEQ_EVENT_CHANPRESS: {
        ChanPressEvent* e = static_cast<ChanPressEvent*>(sev);
        cout << qSetFieldWidth(23) << left << "Channel aftertouch";
        cout << qSetFieldWidth(2) << right << e->getChannel() << qSetFieldWidth(0) << " ";
        cout << qSetFieldWidth(3) << e->getValue();
        break;
    }
    case SND_SEQ_EVENT_PITCHBEND: {
        PitchBendEvent* e = static_cast<PitchBendEvent*>(sev);
        cout << qSetFieldWidth(23) << left << "Pitch bend";
        cout << qSetFieldWidth(2) << right << e->getChannel() << qSetFieldWidth(0) << " ";
        cout << qSetFieldWidth(5) << e->getValue();
        break;
    }
    case SND_SEQ_EVENT_SONGPOS: {
        ValueEvent* e = static_cast<ValueEvent*>(sev);
        cout << qSetFieldWidth(26) << left << "Song position pointer";
        cout << qSetFieldWidth(0) << e->getValue();
        break;
    }
    case SND_SEQ_EVENT_SONGSEL: {
        ValueEvent* e = static_cast<ValueEvent*>(sev);
        cout << qSetFieldWidth(26) << left << "Song select";
        cout << qSetFieldWidth(0) << e->getValue();
        break;
    }
    case SND_SEQ_EVENT_QFRAME: {
        ValueEvent* e = static_cast<ValueEvent*>(sev);
        cout << qSetFieldWidth(26) << left << "MTC quarter frame";
        cout << qSetFieldWidth(0) << e->getValue();
        break;
    }
    case SND_SEQ_EVENT_TIMESIGN: {
        ValueEvent* e = static_cast<ValueEvent*>(sev);
        cout << qSetFieldWidth(26) << left << "SMF time signature";
        cout << hex << qSetFieldWidth(0) << e->getValue();
        cout << dec;
        break;
    }
    case SND_SEQ_EVENT_KEYSIGN: {
        ValueEvent* e = static_cast<ValueEvent*>(sev);
        cout << qSetFieldWidth(26) << left << "SMF key signature";
        cout << qSetFieldWidth(0) << hex << e->getValue();
        cout << dec;
        break;
    }
    case SND_SEQ_EVENT_SETPOS_TICK: {
        QueueControlEvent* e = static_cast<QueueControlEvent*>(sev);
        cout << qSetFieldWidth(26) << left << "Set tick queue pos.";
        cout << e->getQueue();
        break;
    }
    case SND_SEQ_EVENT_SETPOS_TIME: {
        QueueControlEvent* e = static_cast<QueueControlEvent*>(sev);
        cout << qSetFieldWidth(26) << left << "Set rt queue pos.";
        cout << qSetFieldWidth(0) << e->getQueue();
        break;
    }
    case SND_SEQ_EVENT_TEMPO: {
        TempoEvent* e = static_cast<TempoEvent*>(sev);
        cout << qSetFieldWidth(26) << left << "Set queue tempo";
        cout << qSetFieldWidth(3) << right << e->getQueue() << " ";
        cout << qSetFieldWidth(0) << e->getValue();
        break;
    }
    case SND_SEQ_EVENT_QUEUE_SKEW: {
        QueueControlEvent* e = static_cast<QueueControlEvent*>(sev);
        cout << qSetFieldWidth(26) << left << "Queue timer skew";
        cout << qSetFieldWidth(0) << e->getQueue();
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
        cout << qSetFieldWidth(26) << left << "Client start";
        cout << qSetFieldWidth(0) << e->getClient();
        break;
    }
    case SND_SEQ_EVENT_CLIENT_EXIT: {
        ClientEvent* e = static_cast<ClientEvent*>(sev);
        cout << qSetFieldWidth(26) << left << "Client exit";
        cout << qSetFieldWidth(0) << e->getClient();
        break;
    }
    case SND_SEQ_EVENT_CLIENT_CHANGE: {
        ClientEvent* e = static_cast<ClientEvent*>(sev);
        cout << qSetFieldWidth(26) << left << "Client changed";
        cout << qSetFieldWidth(0) << e->getClient();
        break;
    }
    case SND_SEQ_EVENT_PORT_START: {
        PortEvent* e = static_cast<PortEvent*>(sev);
        cout << qSetFieldWidth(26) << left << "Port start";
        cout << qSetFieldWidth(0) << e->getClient() << ":" << e->getPort();
        break;
    }
    case SND_SEQ_EVENT_PORT_EXIT: {
        PortEvent* e = static_cast<PortEvent*>(sev);
        cout << qSetFieldWidth(26) << left << "Port exit";
        cout << qSetFieldWidth(0) << e->getClient() << ":" << e->getPort();
        break;
    }
    case SND_SEQ_EVENT_PORT_CHANGE: {
        PortEvent* e = static_cast<PortEvent*>(sev);
        cout << qSetFieldWidth(26) << left << "Port changed";
        cout << qSetFieldWidth(0) << e->getClient() << ":" << e->getPort();
        break;
    }
    case SND_SEQ_EVENT_PORT_SUBSCRIBED: {
        SubscriptionEvent* e = static_cast<SubscriptionEvent*>(sev);
        cout << qSetFieldWidth(26) << left << "Port subscribed";
        cout << qSetFieldWidth(0) << e->getSenderClient() << ":" << e->getSenderPort() << " -> ";
        cout << e->getDestClient() << ":" << e->getDestPort();
        break;
    }
    case SND_SEQ_EVENT_PORT_UNSUBSCRIBED: {
        SubscriptionEvent* e = static_cast<SubscriptionEvent*>(sev);
        cout << qSetFieldWidth(26) << left << "Port unsubscribed";
        cout << qSetFieldWidth(0) << e->getSenderClient() << ":" << e->getSenderPort() << " -> ";
        cout << e->getDestClient() << ":" << e->getDestPort();
        break;
    }
    case SND_SEQ_EVENT_SYSEX: {
        SysExEvent* e = static_cast<SysExEvent*>(sev);
        cout << qSetFieldWidth(26) << left << "System exclusive" << qSetFieldWidth(0);
        unsigned int i;
        for (i = 0; i < e->getLength(); ++i) {
            cout << hex << (unsigned char) e->getData()[i] << " ";
        }
        cout << dec;
        break;
    }
    case SND_SEQ_EVENT_USR_VAR0: {
        TextEvent* e = static_cast<TextEvent*>(sev);
        cout << qSetFieldWidth(0) << left << "Text (";
        cout << e->getTextType() << ") " << e->getText();
        break;
    }
    default:
        cout << qSetFieldWidth(26) << "Unknown event type";
        cout << qSetFieldWidth(0) << sev->getSequencerType();
    };
    cout << qSetFieldWidth(0) << endl;
    delete sev;
}

void QTestEvents::run()
{
    dumpEvent(new NoteEvent(0, 60, 100, 120));
    dumpEvent(new NoteOnEvent(1, 60, 100));
    dumpEvent(new NoteOffEvent(2, 60, 0));
    dumpEvent(new ControllerEvent(3, 33, 66));
    dumpEvent(new ProgramChangeEvent(4, 123));
    dumpEvent(new KeyPressEvent(5, 60, 124));
    dumpEvent(new ChanPressEvent(6, 111));
    dumpEvent(new PitchBendEvent(7, 1234));
    char sysex[] = {0xf0, 0x41, 0x10, 0x42, 0x12, 0x40, 0, 0x7f, 0, 0x41, 0xf7};
    dumpEvent(new SysExEvent(QByteArray(sysex, sizeof(sysex))));
    QString text = "This can be a copyright, song name, instrument, lyric...";
    TextEvent te(text, 3);
    dumpEvent(new TextEvent(te));
}

int main(int argc, char **argv)
{
    QTestEvents test;
    CmdLineArgs args;
    args.parse(argc, argv);
    test.run();
    return 0;
}
