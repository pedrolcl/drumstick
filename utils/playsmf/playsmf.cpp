/*
    Standard MIDI File player program
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

#include "playsmf.h"
#include "cmdlineargs.h"

#include <signal.h>
#include <QCoreApplication>
#include <QTextStream>
#include <QtAlgorithms>
#include <QFileInfo>
#include <QReadLocker>
#include <QWriteLocker>

static QTextStream cout(stdout, QIODevice::WriteOnly);
static QTextStream cerr(stderr, QIODevice::WriteOnly);

/* ********** *
 * Song class
 * ********** */

static inline bool eventLessThan(const SequencerEvent* s1, const SequencerEvent *s2)
{
    return s1->getTick() < s2->getTick();
}

void Song::sort()
{
    qStableSort(begin(), end(), eventLessThan);
}

void Song::clear()
{
    while (!isEmpty())
        delete takeFirst();
}

Song::~Song()
{
    clear();
}

/* ************* *
 * PlaySMF class
 * ************* */

PlaySMF::PlaySMF() :
    m_division(-1),
    m_portId(-1),
    m_queueId(-1),
    m_initialTempo(-1),
    m_Stopped(true)
{
    m_Client = new MidiClient(this);
    m_Client->open();
    m_Client->setClientName("MIDI Player");

    m_Port = new MidiPort(this);
    m_Port->attach( m_Client );
    m_Port->setPortName("MIDI Player port");
    m_Port->setCapability(SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ);
    m_Port->setPortType(SND_SEQ_PORT_TYPE_APPLICATION);

    m_Queue = m_Client->createQueue();
    m_queueId = m_Queue->getId();
    m_portId = m_Port->getPortId();

    m_engine = new QSmf(this);
    connect(m_engine, SIGNAL(signalSMFHeader(int,int,int)), SLOT(headerEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalSMFNoteOn(int,int,int)), SLOT(noteOnEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalSMFNoteOff(int,int,int)), SLOT(noteOffEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalSMFKeyPress(int,int,int)), SLOT(keyPressEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalSMFCtlChange(int,int,int)), SLOT(ctlChangeEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalSMFPitchBend(int,int)), SLOT(pitchBendEvent(int,int)));
    connect(m_engine, SIGNAL(signalSMFProgram(int,int)), SLOT(programEvent(int,int)));
    connect(m_engine, SIGNAL(signalSMFChanPress(int,int)), SLOT(chanPressEvent(int,int)));
    connect(m_engine, SIGNAL(signalSMFSysex(const QByteArray&)), SLOT(sysexEvent(const QByteArray&)));
    connect(m_engine, SIGNAL(signalSMFText(int,const QString&)), SLOT(textEvent(int,const QString&)));
    connect(m_engine, SIGNAL(signalSMFTempo(int)), SLOT(tempoEvent(int)));
    connect(m_engine, SIGNAL(signalSMFTimeSig(int,int,int,int)), SLOT(timeSigEvent(int,int,int,int)));
    connect(m_engine, SIGNAL(signalSMFKeySig(int,int)), SLOT(keySigEvent(int,int)));
    connect(m_engine, SIGNAL(signalSMFError(const QString&)), SLOT(errorHandler(const QString&)));

    //connect(m_engine, SIGNAL(signalSMFTrackStart()), SLOT(trackStartEvent()));
    //connect(m_engine, SIGNAL(signalSMFTrackEnd()), SLOT(trackEndEvent()));
    //connect(m_engine, SIGNAL(signalSMFendOfTrack()), SLOT(endOfTrackEvent()));
    //connect(m_engine, SIGNAL(signalSMFMetaMisc(int, const QByteArray&)), SLOT(metaMiscEvent(int, const QByteArray&)));
    //connect(m_engine, SIGNAL(signalSMFVariable(const QByteArray&)), SLOT(variableEvent(const QByteArray&)));
    //connect(m_engine, SIGNAL(signalSMFSequenceNum(int)), SLOT(seqNum(int)));
    //connect(m_engine, SIGNAL(signalSMFforcedChannel(int)), SLOT(forcedChannel(int)));
    //connect(m_engine, SIGNAL(signalSMFforcedPort(int)), SLOT(forcedPort(int)));
    //connect(m_engine, SIGNAL(signalSMFSmpte(int,int,int,int,int)), SLOT(smpteEvent(int,int,int,int,int)));
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
    } catch (const SequencerError& err) {
        cerr << "SequencerError exception. Error code: " << err.code()
             << " (" << err.qstrError() << ")" << endl;
        cerr << "Location: " << err.location() << endl;
        throw err;
    }
}

bool PlaySMF::stopped()
{
	QReadLocker locker(&m_mutex);
    return m_Stopped;
}

void PlaySMF::stop()
{
	QWriteLocker locker(&m_mutex);
    m_Stopped = true;
    m_Client->dropOutput();
}

void PlaySMF::shutupSound()
{
    int channel;
    for (channel = 0; channel < 16; ++channel) {
        ControllerEvent ev(channel, MIDI_CTL_ALL_SOUNDS_OFF, 0);
        ev.setSource(m_portId);
        ev.setSubscribers();
        ev.setDirect();
        m_Client->outputDirect(&ev);
    }
    m_Client->drainOutput();
}

void PlaySMF::appendEvent(SequencerEvent* ev)
{
    long tick = m_engine->getCurrentTime();
    ev->setSource(m_portId);
    if (ev->getSequencerType() != SND_SEQ_EVENT_TEMPO) {
        ev->setSubscribers();
    }
    ev->scheduleTick(m_queueId, tick, false);
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

void PlaySMF::noteOnEvent(int chan, int pitch, int vol)
{
    NoteOnEvent* ev = new NoteOnEvent (chan, pitch, vol);
    appendEvent(ev);
}

void PlaySMF::noteOffEvent(int chan, int pitch, int vol)
{
    SequencerEvent* ev = new NoteOffEvent(chan, pitch, vol);
    appendEvent(ev);
}

void PlaySMF::keyPressEvent(int chan, int pitch, int press)
{
    SequencerEvent* ev = new KeyPressEvent (chan, pitch, press);
    appendEvent(ev);
}

void PlaySMF::ctlChangeEvent(int chan, int ctl, int value)
{
    SequencerEvent* ev = new ControllerEvent (chan, ctl, value);
    appendEvent(ev);
}

void PlaySMF::pitchBendEvent(int chan, int value)
{
    SequencerEvent* ev = new PitchBendEvent (chan, value);
    appendEvent(ev);
}

void PlaySMF::programEvent(int chan, int patch)
{
    SequencerEvent* ev = new ProgramChangeEvent (chan, patch);
    appendEvent(ev);
}

void PlaySMF::chanPressEvent(int chan, int press)
{
    SequencerEvent* ev = new ChanPressEvent (chan, press);
    appendEvent(ev);
}

void PlaySMF::sysexEvent(const QByteArray& data)
{
    SysExEvent* ev = new SysExEvent(data);
    appendEvent(ev);
}

void PlaySMF::textEvent(int typ, const QString& data)
{
    dumpStr(QString("Text (%1)").arg(typ), data);
}

void PlaySMF::timeSigEvent(int b0, int b1, int b2, int b3)
{
    dump("--", "Time Signature", QString("%1, %2, %3, %4").arg(b0).arg(b1).arg(b2).arg(b3));
}

void PlaySMF::keySigEvent(int b0, int b1)
{
    dump("--", "Key Signature", QString("%1, %2").arg(b0).arg(b1));
}

void PlaySMF::tempoEvent(int tempo)
{
    if ( m_initialTempo < 0 )
    {
        m_initialTempo = tempo;
    }
    TempoEvent* ev = new TempoEvent(m_queueId, tempo);
    appendEvent(ev);
}

void PlaySMF::errorHandler(const QString& errorStr)
{
    cout << "*** Warning! " << errorStr
         << " at file offset " << m_engine->getFilePos()
         << endl;
}

void PlaySMF::play(QString fileName)
{
    cout << "Reading song: " << fileName << endl;
    cout << "___time ch event__________ data____" << endl;
    m_engine->readFromFile(fileName);
    m_song.sort();
    m_Client->setPoolOutput(100);

    QueueTempo firstTempo = m_Queue->getTempo();
    firstTempo.setPPQ(m_division);
    if (m_initialTempo > 0)
        firstTempo.setTempo(m_initialTempo);
    m_Queue->setTempo(firstTempo);
    m_Client->drainOutput();
    cout << "Starting playback" << endl;
    cout << "Press Ctrl+C to exit" << endl;
    try {
        QListIterator<SequencerEvent*> i(m_song);
        m_Stopped = false;
        m_Queue->start();
        while (!stopped() && i.hasNext()) {
            //m_Client->outputDirect(i.next());
            m_Client->output(i.next());
        }
        if (stopped()) {
            m_Queue->clear();
            shutupSound();
        } else {
            m_Client->drainOutput();
            m_Client->synchronizeOutput();
        }
        m_Queue->stop();
    } catch (const SequencerError& err) {
        cerr << "SequencerError exception. Error code: " << err.code()
             << " (" << err.qstrError() << ")" << endl;
        cerr << "Location: " << err.location() << endl;
        throw err;
    }
}

/*
void PlaySMF::usage()
{
    cout << "Error: wrong parameters" << endl;
    cout << "Usage:" << endl;
    cout << "\tplaysmf PORT FILE.MID" << endl;
}

void PlaySMF::info()
{
    SystemInfo info = m_Client->getSystemInfo();
    cout << "ALSA Sequencer System Info" << endl;
    cout << "\tMax Clients: " << info.getMaxClients() << endl;
    cout << "\tMax Ports: " << info.getMaxPorts() << endl;
    cout << "\tMax Queues: " << info.getMaxQueues() << endl;
    cout << "\tMax Channels: " << info.getMaxChannels() << endl;
    cout << "\tCurrent Queues: " << info.getCurrentQueues() << endl;
    cout << "\tCurrent Clients: " << info.getCurrentClients() << endl;
}
*/

PlaySMF* player = 0;

void signalHandler(int sig)
{
    if (sig == SIGINT)
        qDebug() << "Caught a SIGINT. Exiting";
    else if (sig == SIGTERM)
        qDebug() << "Caught a SIGTERM. Exiting";
    if (player != 0)
        player->stop();
}

int main(int argc, char **argv)
{
    const QString errorstr = "Fatal error from the ALSA sequencer. "
        "This usually happens when the kernel doesn't have ALSA support, "
        "or the device node (/dev/snd/seq) doesn't exists, "
        "or the kernel module (snd_seq) is not loaded. "
        "Please check your ALSA/MIDI configuration.";

    CmdLineArgs args;
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    args.setUsage("[options] port file...");
    args.addRequiredArgument("port", "Destination, MIDI port");
    args.addMultipleArgument("file", "Input SMF(s)");
    args.parse(argc, argv);

    try {
        player = new PlaySMF();
        QVariant port = args.getArgument("port");
        if (!port.isNull())
            player->subscribe(port.toString());

        QVariantList files = args.getArguments("file");
        foreach(const QVariant& f, files) {
            QFileInfo file(f.toString());
            if (file.exists())
                player->play(file.canonicalFilePath());
        }
        //player->info();
    } catch (const SequencerError& ex) {
        cerr << errorstr + " Returned error was: " + ex.qstrError() << endl;
    } catch (...) {
        cerr << errorstr << endl;
    }
    delete player;
    return 0;
}
