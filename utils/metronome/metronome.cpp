/*
    Standard MIDI simple metronome
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

#include "metronome.h"
#include "alsatimer.h"
#include "cmdlineargs.h"

#include <signal.h>
#include <QCoreApplication>
#include <QTextStream>
#include <QtAlgorithms>
#include <QStringList>
#include <QReadLocker>
#include <QWriteLocker>

static QTextStream cout(stdout, QIODevice::WriteOnly);
static QTextStream cerr(stderr, QIODevice::WriteOnly);

/* *************** *
 * Metronome class *
 * *************** */

Metronome::Metronome(QObject *parent) : QObject(parent),
    m_weak_note(METRONOME_WEAK_NOTE),
    m_strong_note(METRONOME_STRONG_NOTE),
    m_weak_velocity(METRONOME_VELOCITY),
    m_strong_velocity(METRONOME_VELOCITY),
    m_program(METRONOME_PROGRAM),
    m_channel(METRONOME_CHANNEL),
    m_volume(METRONOME_VOLUME),
    m_pan(METRONOME_PAN),
    m_resolution(METRONOME_RESOLUTION),
    m_bpm(TEMPO_DEFAULT),
    m_ts_num(RHYTHM_TS_NUM),
    m_ts_div(RHYTHM_TS_DEN),
    m_noteDuration(NOTE_DURATION),
    m_portId(-1),
    m_queueId(-1),
    m_clientId(-1),
    m_Stopped(true)
{
    QLatin1String name("Metronome");
    m_Client = new MidiClient(this);
    m_Client->open();
    m_Client->setClientName(name);
    m_Client->setHandler(this);
    m_Port = new MidiPort(this);
    m_Port->attach( m_Client );
    m_Port->setPortName(name);
    m_Port->setCapability( SND_SEQ_PORT_CAP_READ |
                           SND_SEQ_PORT_CAP_SUBS_READ |
                           SND_SEQ_PORT_CAP_WRITE );
    m_Port->setPortType( SND_SEQ_PORT_TYPE_MIDI_GENERIC |
                         SND_SEQ_PORT_TYPE_APPLICATION );
    m_Queue = m_Client->createQueue(name);
    m_clientId = m_Client->getClientId();
    m_queueId = m_Queue->getId();
    m_portId = m_Port->getPortId();
    m_Port->setTimestamping(true);
    m_Port->setTimestampQueue(m_queueId);
    // Get and apply the best available timer
    TimerId best = Timer::bestGlobalTimerId();
    QueueTimer qtimer;
    qtimer.setId(best);
    m_Queue->setTimer(qtimer);
    // Start sequencer input
    m_Client->setRealTimeInput(false);
    m_Client->startSequencerInput();
}

Metronome::~Metronome()
{
    m_Port->detach();
    m_Client->close();
}

void Metronome::handleSequencerEvent( SequencerEvent *ev )
{
    if (ev->getSequencerType() == SND_SEQ_EVENT_USR0)
        metronome_pattern(ev->getTick() + m_patternDuration);
    delete ev;
}

void Metronome::metronome_event_output(SequencerEvent* ev)
{
    ev->setSource(m_portId);
    ev->setSubscribers();
    ev->setDirect();
    m_Client->outputDirect(ev);
}

void Metronome::sendControlChange(int cc, int value)
{
    ControllerEvent ev(m_channel, cc, value);
    metronome_event_output(&ev);
}

void Metronome::sendInitialControls()
{
    metronome_set_program();
    metronome_set_controls();
    metronome_set_tempo();
}

void Metronome::metronome_set_program()
{
    ProgramChangeEvent ev(m_channel, m_program);
    metronome_event_output(&ev);
}

void Metronome::metronome_schedule_event(SequencerEvent* ev, int tick, bool lb)
{
    ev->setSource(m_portId);
    if (lb) // loop back
        ev->setDestination(m_clientId, m_portId);
    else
        ev->setSubscribers();
    ev->scheduleTick(m_queueId, tick, false);
    m_Client->outputDirect(ev);
}

void Metronome::metronome_note(int note, int tick)
{
    NoteEvent ev(m_channel, note, METRONOME_VELOCITY, m_noteDuration);
    metronome_schedule_event(&ev, tick, false);
}

void Metronome::metronome_echo(int tick)
{
    SystemEvent ev(SND_SEQ_EVENT_USR0);
    metronome_schedule_event(&ev, tick, true);
}

void Metronome::metronome_pattern(int tick)
{
    int j, t, duration;
    t = tick;
    duration = m_resolution * 4 / m_ts_div;
    for (j = 0; j < m_ts_num; j++) {
        metronome_note(j ? m_weak_note : m_strong_note, t);
        t += duration;
    }
    metronome_echo(t);
}

void Metronome::metronome_set_tempo()
{
    QueueTempo t = m_Queue->getTempo();
    t.setPPQ(m_resolution);
    t.setNominalBPM(m_bpm);
    m_Queue->setTempo(t);
    m_Client->drainOutput();
}

void Metronome::metronome_set_controls()
{
    sendControlChange(MIDI_CTL_MSB_MAIN_VOLUME, m_volume);
    sendControlChange(MIDI_CTL_MSB_PAN, m_pan);
}

void Metronome::subscribe(const QString& portName)
{
    m_Port->subscribeTo(portName);
}

bool Metronome::stopped()
{
	QReadLocker locker(&m_mutex);
    return m_Stopped;
}

void Metronome::stop()
{
	QWriteLocker locker(&m_mutex);
    m_Stopped = true;
    m_Client->dropOutput();
}

void Metronome::shutupSound()
{
    sendControlChange( MIDI_CTL_ALL_NOTES_OFF, 0 );
    sendControlChange( MIDI_CTL_ALL_SOUNDS_OFF, 0 );
}

void Metronome::play(QString tempo)
{
    bool ok;
    m_Stopped = false;
    m_patternDuration = m_resolution * 4 / m_ts_div * m_ts_num;
    m_bpm = tempo.toInt(&ok);
    if (!ok) m_bpm = TEMPO_DEFAULT;
    cout << "Metronome playing. " << m_bpm << " bpm" << endl;
    cout << "Press Ctrl+C to exit" << endl;
    try {
        sendInitialControls();
        m_Queue->start();
        metronome_pattern(0);
        metronome_pattern(m_patternDuration);
        while (!stopped())
            sleep(1);
    } catch (const SequencerError& err) {
        cerr << "SequencerError exception. Error code: " << err.code()
             << " (" << err.qstrError() << ")" << endl;
        cerr << "Location: " << err.location() << endl;
    }
}

/*
void Metronome::usage()
{
    cout << "Error: wrong parameters" << endl;
    cout << "Usage:" << endl;
    cout << "   metronome PORT BPM" << endl;
}

void Metronome::info()
{
    SystemInfo info = m_Client->getSystemInfo();
    cout << "ALSA Sequencer System Info" << endl;
    cout << "Max Clients: " << info.getMaxClients() << endl;
    cout << "Max Ports: " << info.getMaxPorts() << endl;
    cout << "Max Queues: " << info.getMaxQueues() << endl;
    cout << "Max Channels: " << info.getMaxChannels() << endl;
    cout << "Current Queues: " << info.getCurrentQueues() << endl;
    cout << "Current Clients: " << info.getCurrentClients() << endl;
}
*/

Metronome* metronome = 0;

void signalHandler(int sig)
{
    if (sig == SIGINT)
        qDebug() << "Caught a SIGINT. Exiting";
    else if (sig == SIGTERM)
        qDebug() << "Caught a SIGTERM. Exiting";
    if (metronome != 0) {
        metronome->stop();
        metronome->shutupSound();
    }
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

    args.setUsage("[options] port [bpm]");
    args.addRequiredArgument("port", "Destination, MIDI port identifier");
    args.addOptionalArgument("bpm", "Tempo, in beats per minute (default=120)");
    args.parse(argc, argv);

    try {
        metronome = new Metronome();
        QVariant port = args.getArgument("port");
        if (!port.isNull())
            metronome->subscribe(port.toString());
        QVariant bpm = args.getArgument("bpm");
        metronome->play(bpm.toString());
    } catch (const SequencerError& ex) {
        cerr << errorstr + " Returned error was: " + ex.qstrError() << endl;
    } catch (...) {
        cerr << errorstr << endl;
    }
    delete metronome;
    return 0;
}
