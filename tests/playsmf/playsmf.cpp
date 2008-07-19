/*
    Standard MIDI File player program 
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

#include "playsmf.h"
#include <signal.h>
#include <QApplication>
#include <QTextStream>
#include <QtAlgorithms>

static QTextStream cout(stdout, QIODevice::WriteOnly); 
static QTextStream cerr(stderr, QIODevice::WriteOnly); 

static inline bool eventLessThan(const SequencerEvent &s1, const SequencerEvent &s2)
{
    return s1.getTick() < s2.getTick();
}

void Song::sort() 
{
    qStableSort(begin(), end(), eventLessThan);
}

PlaySMF::PlaySMF() :
    m_division(-1),
    m_portId(-1),
    m_queueId(-1),
    m_initialTempo(-1),
    m_Stopped(true)
    {
    m_Client = new MidiClient(this);
    m_Port = new MidiPort(this);
    m_Client->setOpenMode(SND_SEQ_OPEN_DUPLEX);
    m_Client->setBlockMode(false);
    m_Client->open();
    m_Client->setClientName("MIDI Player");

    m_Port->setMidiClient(m_Client);
    m_Port->setPortName("MIDI Player port");
    m_Port->setCapability(SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ);
    m_Port->setPortType(SND_SEQ_PORT_TYPE_APPLICATION);
    m_Port->attach();

    m_Queue = m_Client->createQueue();
    m_queueId = m_Queue->getId();
    m_portId = m_Port->getPortId();

    m_engine = new QSmf(this);
    connect(m_engine, SIGNAL(signalSMFHeader(int,int,int)), SLOT(headerEvent(int,int,int)));
    //connect(m_engine, SIGNAL(signalSMFTrackStart()), SLOT(trackStartEvent()));
    //connect(m_engine, SIGNAL(signalSMFTrackEnd()), SLOT(trackEndEvent()));
    //connect(m_engine, SIGNAL(signalSMFendOfTrack()), SLOT(endOfTrackEvent()));
    connect(m_engine, SIGNAL(signalSMFNoteOn(int,int,int)), SLOT(noteOnEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalSMFNoteOff(int,int,int)), SLOT(noteOffEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalSMFKeyPress(int,int,int)), SLOT(keyPressEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalSMFCtlChange(int,int,int)), SLOT(ctlChangeEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalSMFPitchBend(int,int)), SLOT(pitchBendEvent(int,int)));
    connect(m_engine, SIGNAL(signalSMFProgram(int,int)), SLOT(programEvent(int,int)));
    connect(m_engine, SIGNAL(signalSMFChanPress(int,int)), SLOT(chanPressEvent(int,int)));
    connect(m_engine, SIGNAL(signalSMFSysex(const QByteArray&)), SLOT(sysexEvent(const QByteArray&)));
    //connect(m_engine, SIGNAL(signalSMFMetaMisc(int, const QByteArray&)), SLOT(metaMiscEvent(int, const QByteArray&)));
    //connect(m_engine, SIGNAL(signalSMFVariable(const QByteArray&)), SLOT(variableEvent(const QByteArray&)));
    //connect(m_engine, SIGNAL(signalSMFSequenceNum(int)), SLOT(seqNum(int)));
    //connect(m_engine, SIGNAL(signalSMFforcedChannel(int)), SLOT(forcedChannel(int)));
    //connect(m_engine, SIGNAL(signalSMFforcedPort(int)), SLOT(forcedPort(int)));
    connect(m_engine, SIGNAL(signalSMFText(int,const QString&)), SLOT(textEvent(int,const QString&)));
    //connect(m_engine, SIGNAL(signalSMFTimeSig(int,int,int,int)), SLOT(timeSigEvent(int,int,int,int)));
    //connect(m_engine, SIGNAL(signalSMFSmpte(int,int,int,int,int)), SLOT(smpteEvent(int,int,int,int,int)));
    //connect(m_engine, SIGNAL(signalSMFKeySig(int,int)), SLOT(keySigEvent(int,int)));
    connect(m_engine, SIGNAL(signalSMFTempo(int)), SLOT(tempoEvent(int)));
    connect(m_engine, SIGNAL(signalSMFError(const QString&)), SLOT(errorHandler(const QString&)));
    }

PlaySMF::~PlaySMF()
{
    m_Port->detach();
    m_Client->close();
}

void PlaySMF::subscribe(const QString& portName)
{
    try {
        qDebug() << "Trying to subscribe to " << portName.toLocal8Bit().data();
        m_Port->subscribeTo(portName);
    } catch (FatalError *err) {
        cerr << "FatalError exception. Error code: " << err->code() 
        << " (" << err->qstrError() << ")" << endl;
        cerr << "Location: " << err->what() << endl;
        throw err;
    }
}

bool PlaySMF::stopped()
{
    m_mutex.lock();
    bool bTmp = m_Stopped;
    m_mutex.unlock();
    return bTmp;
}

void PlaySMF::stop()
{
    m_mutex.lock();
    m_Stopped = true;
    m_mutex.unlock();
}

void PlaySMF::shutupSound()
{
    int channel;
    for (channel = 0; channel < 16; ++channel)
    {
        ControllerEvent ev(channel, MIDI_CTL_ALL_SOUNDS_OFF, 0);
        ev.setSource(m_portId);
        ev.setSubscribers();
        ev.setDirect();
        m_Client->outputDirect(&ev);
    }
    m_Client->drainOutput();
}

void PlaySMF::appendEvent(SequencerEvent& ev)
{
    long tick = m_engine->getCurrentTime();
    ev.setSource(m_portId);
    if (ev.getSequencerType() != SND_SEQ_EVENT_TEMPO) {
        ev.setSubscribers();
    }
    ev.scheduleTick(m_queueId, tick, false);
    m_song.append(ev);
}

void PlaySMF::dump(const QString& chan, const QString& event,
                   const QString& data)
{
    cout << right << qSetFieldWidth(7) << m_engine->getCurrentTime();
    cout << qSetFieldWidth(3) << chan;
    cout << qSetFieldWidth(0) << left << " ";
    cout << qSetFieldWidth(15) << event;
    cout << qSetFieldWidth(0) << " " << data << endl;
}

void PlaySMF::dumpStr(const QString& event, const QString& data)
{
    cout << right << qSetFieldWidth(7) << m_engine->getCurrentTime();
    cout << qSetFieldWidth(3) << "--";
    cout << qSetFieldWidth(0) << left << " ";
    cout << qSetFieldWidth(15) << event;
    cout << qSetFieldWidth(0) << " " << data << endl;
}

void PlaySMF::headerEvent(int format, int ntrks, int division)
{
    m_division = division;
    dumpStr("SMF Header", QString("Format=%1, Tracks=%2, Division=%3").
            arg(format).arg(ntrks).arg(division));
}

/*void PlaySMF::trackStartEvent()
{
    dumpStr("Track", "Start");
}

void PlaySMF::trackEndEvent()
{
    dumpStr("Track", "End");
}

void PlaySMF::endOfTrackEvent()
{
    dumpStr("Meta Event", "End Of Track");
}*/

void PlaySMF::noteOnEvent(int chan, int pitch, int vol)
{
    NoteOnEvent ev(chan, pitch, vol);
    appendEvent(ev);
}

void PlaySMF::noteOffEvent(int chan, int pitch, int vol)
{
    NoteOffEvent ev(chan, pitch, vol);
    appendEvent(ev);
}

void PlaySMF::keyPressEvent(int chan, int pitch, int press)
{
    KeyPressEvent ev(chan, pitch, press);
    appendEvent(ev);
}

void PlaySMF::ctlChangeEvent(int chan, int ctl, int value)
{
    ControllerEvent ev(chan, ctl, value);
    appendEvent(ev);
}

void PlaySMF::pitchBendEvent(int chan, int value)
{
    PitchBendEvent ev(chan, value);
    appendEvent(ev);
}

void PlaySMF::programEvent(int chan, int patch)
{
    ProgramChangeEvent ev(chan, patch);
    appendEvent(ev);
}

void PlaySMF::chanPressEvent(int chan, int press)
{
    ChanPressEvent ev(chan, press);
    appendEvent(ev);
}

void PlaySMF::sysexEvent(const QByteArray& data)
{
    SysExEvent ev(data.length(), (char *)data.data());
    appendEvent(ev);
}

/*void PlaySMF::variableEvent(const QByteArray& data)
{
    int j;
    QString s;
    for (j = 0; j < data.count(); ++j)
        s.append(QString("%1 ").arg(data[j] & 0xff, 2, 16));
    dumpStr("Variable event", s);
}

void PlaySMF::metaMiscEvent(int typ, const QByteArray& data)
{
    int j;
    QString s = QString("type=%1 ").arg(typ);
    for (j = 0; j < data.count(); ++j)
        s.append(QString("%1 ").arg(data[j] & 0xff, 2, 16));
    dumpStr("Meta", s);
}

void PlaySMF::seqNum(int seq)
{  
    dump("--", "Sequence num.", QString::number(seq));
}

void PlaySMF::forcedChannel(int channel)
{
    dump("--", "Forced channel", QString::number(channel));
}

void PlaySMF::forcedPort(int port)
{
    dump("--", "Forced port", QString::number(port));
}*/

void PlaySMF::textEvent(int typ, const QString& data)
{
    dumpStr(QString("Text (%1)").arg(typ), data);
}

/*void PlaySMF::smpteEvent(int b0, int b1, int b2, int b3, int b4)
{
    dump("--", "SMPTE", QString("%1, %2, %3, %4, %5").arg(b0).arg(b1).arg(b2).arg(b3).arg(b4));
}

void PlaySMF::timeSigEvent(int b0, int b1, int b2, int b3)
{
    dump("--", "Time Signature", QString("%1, %2, %3, %4").arg(b0).arg(b1).arg(b2).arg(b3));
}

void PlaySMF::keySigEvent(int b0, int b1)
{
    dump("--", "Key Signature", QString("%1, %2").arg(b0).arg(b1));
}*/

void PlaySMF::tempoEvent(int tempo)
{
    if ( m_initialTempo < 0 )
    {
        m_initialTempo = tempo;
    }
    //dump("--", "Tempo", QString::number(tempo));
    TempoEvent ev(m_queueId, tempo);
    appendEvent(ev);
}

void PlaySMF::errorHandler(const QString& errorStr)
{
    cerr << errorStr << endl;
    exit(1);
}

void PlaySMF::play(QString fileName)
{  
    cout << "Reading song: " << fileName << endl;
    cout << "___time ch event__________ data____" << endl;
    m_engine->readFromFile(fileName);
    m_song.sort();
    QueueTempo* firstTempo = m_Queue->getTempo();
    firstTempo->setPPQ(m_division);
    if (m_initialTempo > 0)
        firstTempo->setTempo(m_initialTempo);
    m_Queue->setTempo(firstTempo);
    m_Client->drainOutput();
    cout << "Starting playback" << endl;
    cout << "Press Ctrl+C to exit" << endl;
    try {
        QListIterator<SequencerEvent> i(m_song);
        m_Stopped = false;
        m_Queue->start();
        while (!stopped() && i.hasNext()) {
            SequencerEvent ev = i.next();
            m_Client->output(&ev);
        }
        if (!stopped()) {
            m_Client->drainOutput();
        }
        if (stopped()) {
            m_Queue->clear();
            shutupSound();
        }
        if (!stopped()) {
            m_Client->synchronizeOutput();
        }
        m_Queue->stop();
    } catch (FatalError *err) {
        cerr << "FatalError exception. Error code: " << err->code() 
        << " (" << err->qstrError() << ")" << endl;
        cerr << "Location: " << err->what() << endl;
        throw err;
    }
}

void PlaySMF::usage()
{
    cout << "Error: wrong parameters" << endl;
    cout << "Usage:" << endl;
    cout << "\tplaysmf PORT FILE.MID" << endl;
}

PlaySMF *player;

void signalHandler(int sig)
{
    if (sig == SIGINT)
        qDebug() << "Caught a SIGINT. Exiting";
    else if (sig == SIGTERM)
        qDebug() << "Caught a SIGTERM. Exiting";
    player->stop();
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv, false);
    player = new PlaySMF();
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    if (app.argc() == 3) {
        QString portName(app.argv()[1]);
        player->subscribe(portName);
        QString fileName(app.argv()[2]);
        player->play(fileName);
    } else {
        player->usage();
    }
}
