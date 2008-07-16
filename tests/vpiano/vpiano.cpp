/*
    Virtual Piano test using the MIDI Sequencer C++ library 
    Copyright (C) 2006-2008, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along 
    with this program; if not, write to the Free Software Foundation, Inc., 
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.    
*/

#include <QDebug>
#include "vpiano.h"

VPiano::VPiano( QWidget * parent, Qt::WindowFlags flags )
    : QMainWindow(parent, flags),
    m_portId(-1),
    m_queueId(-1),
    m_Client(0),
    m_Port(0),
    m_Queue(0)
{
    ui.setupUi(this);
    ui.statusBar->hide();

    m_Client = new MidiClient(this);
    m_Client->setOpenMode(SND_SEQ_OPEN_DUPLEX);
    m_Client->setBlockMode(false);
    m_Client->open();
    m_Client->setClientName("Virtual Piano");
#ifdef USE_QEVENTS
    m_Client->addSubscriber(this);
    m_Client->setEventsEnabled(true);
#else USE_QEVENTS // using signals instead
    connect(m_Client, SIGNAL(eventReceived(SequencerEvent*)), SLOT(sequencerEvent(SequencerEvent*)));
#endif  

    m_Port = new MidiPort(this);
    m_Port->setMidiClient(m_Client);
    m_Port->setPortName("Virtual Piano port");
    m_Port->setCapability( SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ |
                           SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE );
    m_Port->setPortType( SND_SEQ_PORT_TYPE_APPLICATION );
    m_Port->attach();

    m_Queue = m_Client->createQueue();
    m_queueId = m_Queue->getId();
    m_portId = m_Port->getPortInfo()->getPort();

    connect(ui.actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui.actionAbout, SIGNAL(triggered()), SLOT(slotAbout()));
    connect(ui.actionConnections, SIGNAL(triggered()), SLOT(slotConnections()));
    connect(ui.actionPreferences, SIGNAL(triggered()), SLOT(slotPreferences()));
    connect(ui.pianokeybd, SIGNAL(noteOn(int)), SLOT(slotNoteOn(int)));
    connect(ui.pianokeybd, SIGNAL(noteOff(int)), SLOT(slotNoteOff(int)));
    connect(m_Port, SIGNAL(subscribed(MidiPort*,Subscription*)), SLOT(slotSubscription(MidiPort*,Subscription*)));

    m_Port->subscribeFromAnnounce();
    m_Client->startSequencerInput();
    m_Queue->start();
    qDebug() << "VPiano MainWindow Constructor Done" << endl;
}

VPiano::~VPiano()
{
    m_Queue->stop();
    m_Client->stopSequencerInput();
    m_Port->detach();
    m_Client->close();
    qDebug() << "Cheers!";
}

void VPiano::slotNoteOn(int midiNote)
{
    int chan = dlgPreferences.getOutChannel();
    int vel = dlgPreferences.getVelocity();
    NoteOnEvent ev(chan, midiNote, vel);
    ev.setSource(m_portId);
    ev.setSubscribers();
    ev.setDirect();
    m_Client->outputDirect(&ev);
}
void VPiano::slotNoteOff(int midiNote)
{
    int chan = dlgPreferences.getOutChannel();
    int vel = dlgPreferences.getVelocity();
    NoteOffEvent ev(chan, midiNote, vel);
    ev.setSource(m_portId);
    ev.setSubscribers();
    ev.setDirect();
    m_Client->outputDirect(&ev);
}

void VPiano::displayEvent(SequencerEvent *ev)
{
    try {
        switch (ev->getSequencerType()) {
        case SND_SEQ_EVENT_NOTEON: {
            NoteOnEvent* e = dynamic_cast<NoteOnEvent*>(ev);
            if ((e != NULL) && (dlgPreferences.getInChannel() == e->getChannel())) {
                int note = e->getKey();
                ui.pianokeybd->showNoteOn(note);
                //qDebug() << "NoteOn" << note;
            }
            break;
        }
        case SND_SEQ_EVENT_NOTEOFF: {
            NoteOffEvent* e = dynamic_cast<NoteOffEvent*>(ev);
            if ((e != NULL) && (dlgPreferences.getInChannel() == e->getChannel())) {
                int note = e->getKey();
                ui.pianokeybd->showNoteOff(note);
                //qDebug() << "NoteOff" << note;
            }
            break;
        }
        default:
            break;
        }
    } catch (FatalError *err) {
        qDebug() << "FatalError exception. Error code: " << err->code() 
        << " (" << err->qstrError() << ")";
        qDebug() << "Location: " << err->what();
        throw err;
    }
}

#ifdef USE_QEVENTS
void VPiano::customEvent(QEvent *ev)
{
    if (ev->type() != SequencerEventType) 
        return;
    SequencerEvent* sev = dynamic_cast<SequencerEvent*>(ev);
    if (sev != NULL) {
        displayEvent( sev );
    }
}
#else
void 
VPiano::sequencerEvent(SequencerEvent *ev)
{
    displayEvent( ev );
    delete ev;
}
#endif

void VPiano::slotSubscription(MidiPort*, Subscription* subs)
{
    qDebug() << "Subscription made from" << subs->getSender()->client 
             << ":" << subs->getSender()->port;
    subs->setQueue(m_Client->getQueue()->getId());
}

void VPiano::subscribe(const QString& portName)
{
    try {
        qDebug() << "Trying to subscribe" << portName.toLocal8Bit().data();
        m_Port->subscribeFrom(portName);
    } catch (FatalError *err) {
        qDebug() << "FatalError exception. Error code: " << err->code() 
                 << " (" << err->qstrError() << ")";
        qDebug() << "Location: " << err->what();
        throw err;
    }
}

void VPiano::slotAbout()
{
    dlgAbout.exec();
}

void VPiano::slotConnections()
{
    dlgConnections.exec();
}

void VPiano::slotPreferences()
{
    if (dlgPreferences.exec() == QDialog::Accepted) {
        if (ui.pianokeybd->baseOctave() != dlgPreferences.getBaseOctave()) {
            ui.pianokeybd->setBaseOctave(dlgPreferences.getBaseOctave());
        }
        if (ui.pianokeybd->numOctaves() != dlgPreferences.getNumOctaves()) {
            ui.pianokeybd->setNumOctaves(dlgPreferences.getNumOctaves());
        }
    }
}
