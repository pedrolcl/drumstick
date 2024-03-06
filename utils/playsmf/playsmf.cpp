/*
    Standard MIDI File player program
    Copyright (C) 2006-2024, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "playsmf.h"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QReadLocker>
#include <QTextStream>
#include <QWriteLocker>
#include <QtAlgorithms>
#include <csignal>
#include <drumstick/sequencererror.h>

DISABLE_WARNING_PUSH
DISABLE_WARNING_DEPRECATED_DECLARATIONS

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#define right Qt::right
#define left Qt::left
#define endl Qt::endl
#endif

QTextStream cout(stdout, QIODevice::WriteOnly);
QTextStream cerr(stderr, QIODevice::WriteOnly);

/* ********** *
 * Song class
 * ********** */
using namespace drumstick;
using namespace ALSA;
using namespace File;

static inline bool eventLessThan(const SequencerEvent* s1, const SequencerEvent *s2)
{
    return s1->getTick() < s2->getTick();
}

void Song::sort()
{
    std::sort(begin(), end(), eventLessThan);
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
    connect(m_engine, &QSmf::signalSMFHeader, this, &PlaySMF::headerEvent);
    connect(m_engine, &QSmf::signalSMFNoteOn, this, &PlaySMF::noteOnEvent);
    connect(m_engine, &QSmf::signalSMFNoteOff, this, &PlaySMF::noteOffEvent);
    connect(m_engine, &QSmf::signalSMFKeyPress, this, &PlaySMF::keyPressEvent);
    connect(m_engine, &QSmf::signalSMFCtlChange, this, &PlaySMF::ctlChangeEvent);
    connect(m_engine, &QSmf::signalSMFPitchBend, this, &PlaySMF::pitchBendEvent);
    connect(m_engine, &QSmf::signalSMFProgram, this, &PlaySMF::programEvent);
    connect(m_engine, &QSmf::signalSMFChanPress, this, &PlaySMF::chanPressEvent);
    connect(m_engine, &QSmf::signalSMFSysex, this, &PlaySMF::sysexEvent);
    connect(m_engine, &QSmf::signalSMFText, this, &PlaySMF::textEvent);
    connect(m_engine, &QSmf::signalSMFTempo, this, &PlaySMF::tempoEvent);
    connect(m_engine, &QSmf::signalSMFTimeSig, this, &PlaySMF::timeSigEvent);
    connect(m_engine, &QSmf::signalSMFKeySig, this, &PlaySMF::keySigEvent);
    connect(m_engine, &QSmf::signalSMFError, this, &PlaySMF::errorHandler);
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
        throw;
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
        ev.setSource(static_cast<unsigned char>(m_portId));
        ev.setSubscribers();
        ev.setDirect();
        m_Client->outputDirect(&ev);
    }
    m_Client->drainOutput();
}

void PlaySMF::appendEvent(SequencerEvent* ev)
{
    long tick = m_engine->getCurrentTime();
    ev->setSource(static_cast<unsigned char>(m_portId));
    if (ev->getSequencerType() != SND_SEQ_EVENT_TEMPO) {
        ev->setSubscribers();
    }
    ev->scheduleTick(m_queueId, static_cast<int>(tick), false);
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
        firstTempo.setTempo(static_cast<unsigned int>(m_initialTempo));
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
        throw;
    }
}

static PlaySMF* player = nullptr;

void signalHandler(int sig)
{
    if (sig == SIGINT)
        qDebug() << "Caught a SIGINT. Exiting";
    else if (sig == SIGTERM)
        qDebug() << "Caught a SIGTERM. Exiting";
    if (player != nullptr)
        player->stop();
}

int main(int argc, char **argv)
{
    const QString PGM_NAME = QStringLiteral("drumstick-playsmf");
    const QString PGM_DESCRIPTION = QStringLiteral("Drumstick command line MIDI file player");
    const QString ERRORSTR = QStringLiteral("Fatal error from the ALSA sequencer. "
        "This usually happens when the kernel doesn't have ALSA support, "
        "or the device node (/dev/snd/seq) doesn't exists, "
        "or the kernel module (snd_seq) is not loaded. "
        "Please check your ALSA/MIDI configuration.");

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(PGM_NAME);
    QCoreApplication::setApplicationVersion(QStringLiteral(QT_STRINGIFY(VERSION)));

    QCommandLineParser parser;
    parser.setApplicationDescription(PGM_DESCRIPTION);
    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();
    QCommandLineOption portOption({"p","port"}, "Destination, MIDI port.", "client:port");
    parser.addOption(portOption);
    parser.addPositionalArgument("file", "Input SMF File(s).", "files...");
    parser.process(app);

    if (parser.isSet(versionOption) || parser.isSet(helpOption)) {
        return 0;
    }

    try {
        player = new PlaySMF();
        if (parser.isSet(portOption)) {
            QString port = parser.value(portOption);
            player->subscribe(port);
        } else {
            cerr << "Port argument is missing" << endl;
            parser.showHelp();
        }
        QStringList files = parser.positionalArguments();
        if (files.isEmpty()) {
            cerr << "No input files" << endl;
            parser.showHelp();
        }
        foreach(const QString& f, files) {
            QFileInfo file(f);
            if (file.exists())
                player->play(file.canonicalFilePath());
        }
    } catch (const SequencerError& ex) {
        cerr << ERRORSTR << " Returned error was: " << ex.qstrError() << endl;
    } catch (...) {
        cerr << ERRORSTR << endl;
    }
    delete player;
    return 0;
}

DISABLE_WARNING_POP
