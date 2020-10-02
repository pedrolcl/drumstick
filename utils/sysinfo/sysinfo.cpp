/*
    MIDI Sequencer C++ library 
    Copyright (C) 2006-2020, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QTextStream>
#include <QStringList>
#include <QCommandLineParser>

#include <drumstick/alsatimer.h>
#include <drumstick/alsaqueue.h>
#include <drumstick/alsaclient.h>
#include <drumstick/subscription.h>
#include <drumstick/sequencererror.h>

QString PGM_NAME = QStringLiteral("drumstick-sysinfo");
QString PGM_DESCRIPTION = QStringLiteral("ALSA Sequencer System Info");
QTextStream cout(stdout, QIODevice::WriteOnly);
QTextStream cerr(stderr, QIODevice::WriteOnly);

using namespace drumstick::ALSA;

void queryTimers()
{
    cout << endl << "ALSA Timers" << endl;
    TimerQuery* query = new TimerQuery("hw", 0);
    cout << "type___ Name________________ c/s/C/D/S Freq." << endl;
    TimerIdList lst = query->getTimers();
    TimerIdList::ConstIterator it;
    for( it = lst.constBegin(); it != lst.constEnd(); ++it )
    {
        TimerId id = *it;
        try {
			Timer* timer = new Timer(id, SND_TIMER_OPEN_NONBLOCK);
			TimerInfo info = timer->getTimerInfo();
			cout << qSetFieldWidth(8) << left << info.getId()
				 << qSetFieldWidth(20) << left << info.getName().left(20)
				 << qSetFieldWidth(0) << " "
				 << id.getClass() << "/" << id.getSlaveClass() << "/"
				 << id.getCard() << "/" << id.getDevice() << "/"
				 << id.getSubdevice() << " ";
			if( info.isSlave() ) {
				cout << "SLAVE";
			} else {
				long freq = info.getFrequency();
				cout << freq << " Hz";
			}
			cout << endl;
			delete timer;
        } catch (const SequencerError& err) {
        	cerr << "Error opening timer:"  << err.qstrError();
        }
    }
    delete query;
}

void queryQueues(MidiClient* c)
{
    cout << endl << "ALSA Queues" << endl;    
    cout << "id Queue_Name__________ Timer_Name__________ owner status "
         << "  state PPQ  Tempo BPM   Ticks Time" << endl; 
    QList<int> queues = c->getAvailableQueues();
    foreach( int q, queues ) {
        MidiQueue* queue = new MidiQueue(c, q);
        if (queue != nullptr) {
            QueueInfo qinfo = queue->getInfo();
            QueueStatus qsts = queue->getStatus();
            QueueTempo qtmp = queue->getTempo();
            QueueTimer qtmr = queue->getTimer();
            TimerId tid(qtmr.getId());
            QString tname;
            try {
				Timer* timer = new Timer(tid, SND_TIMER_OPEN_NONBLOCK);
				TimerInfo tinfo = timer->getTimerInfo();
				tname = tinfo.getName();
				delete timer;
            } catch (...) {
            	tname = "inaccessible";
            }
			cout << qSetFieldWidth(3)  << left << qinfo.getId()
				 << qSetFieldWidth(20) << qinfo.getName().left(20)
				 << qSetFieldWidth(0)  << " "
				 << qSetFieldWidth(20) << tname.left(20)
				 << qSetFieldWidth(6)  << right << qinfo.getOwner()
				 << qSetFieldWidth(7)  << (qinfo.isLocked() ? "locked" : "free")
				 << qSetFieldWidth(8)  << (qsts.isRunning() ? "running" : "stopped")
				 << qSetFieldWidth(4)  << qtmp.getPPQ()
				 << qSetFieldWidth(7)  << qtmp.getRealBPM()
				 << qSetFieldWidth(4)  << qtmp.getNominalBPM()
				 << qSetFieldWidth(8)  << qsts.getTickTime()
				 << qSetFieldWidth(0)  << " " << qsts.getClockTime()
				 << endl;
            delete queue;
        }
    }
}

QString clientTypeName(snd_seq_client_type_t ctype)
{
    if (ctype == SND_SEQ_USER_CLIENT)
        return "User";
    if (ctype == SND_SEQ_KERNEL_CLIENT)
        return "Kernel";
    return "Unknown";
}

QString portTypeNames(int ptype)
{
    QStringList lst;
    if ((ptype & SND_SEQ_PORT_TYPE_HARDWARE) != 0)
        lst << "Hardware";
    if ((ptype & SND_SEQ_PORT_TYPE_SOFTWARE) != 0)
        lst << "Software";
    if ((ptype & SND_SEQ_PORT_TYPE_PORT) != 0)
        lst << "Port";
    if ((ptype & SND_SEQ_PORT_TYPE_DIRECT_SAMPLE) != 0)
        lst << "Direct Sample";
    if ((ptype & SND_SEQ_PORT_TYPE_MIDI_GENERIC) != 0)
        lst << "MIDI Generic";
    if ((ptype & SND_SEQ_PORT_TYPE_MIDI_GM) != 0)
        lst << "GM";
    if ((ptype & SND_SEQ_PORT_TYPE_MIDI_GM2) != 0)
        lst << "GM2";
    if ((ptype & SND_SEQ_PORT_TYPE_MIDI_GS) != 0)
        lst << "GS";
    if ((ptype & SND_SEQ_PORT_TYPE_MIDI_MT32) != 0)
        lst << "MT32";
    if ((ptype & SND_SEQ_PORT_TYPE_MIDI_XG) != 0)
        lst << "XG";
    if ((ptype & SND_SEQ_PORT_TYPE_SAMPLE) != 0)
        lst << "Sample";
    if ((ptype & SND_SEQ_PORT_TYPE_SPECIFIC) != 0)
        lst << "Specific";
    if ((ptype & SND_SEQ_PORT_TYPE_SYNTH) != 0)
        lst << "Synth";
    if ((ptype & SND_SEQ_PORT_TYPE_APPLICATION) != 0)
        lst << "Application";
    if ((ptype & SND_SEQ_PORT_TYPE_SYNTHESIZER) != 0)
        lst << "Synthesizer";
    return " (" + lst.join(", ") + ")";
}

QString subsNames(SubscribersList& subs)
{
    QStringList lst;
    foreach( Subscriber s, subs ) {
        QString sname = QString("%1:%2").arg(static_cast<int>(s.getAddr()->client))
                                        .arg(static_cast<int>(s.getAddr()->port));
        lst << sname;
    }
    return lst.join(", ");
}

void queryClients(MidiClient* c)
{
    cout << endl << "ALSA Sequencer clients" << endl;
    ClientInfoList clients = c->getAvailableClients();
    foreach( ClientInfo cinfo, clients ) {
        PortInfoList plist = cinfo.getPorts();
        cout << "Client " << qSetFieldWidth(4) << left << cinfo.getClientId()
             << qSetFieldWidth(0) << " : \"" << cinfo.getName() << "\" ["
             << clientTypeName(cinfo.getClientType())
             << "]" << endl;
        foreach( PortInfo pinfo, plist ) {
            SubscribersList to = pinfo.getReadSubscribers();
            SubscribersList from = pinfo.getWriteSubscribers();
            cout << "  Port " << qSetFieldWidth(4) << right << pinfo.getPort()
                 << qSetFieldWidth(0) << " : \"" << pinfo.getName() << "\""
                 << (pinfo.getType() != 0 ? portTypeNames(pinfo.getType()) : "")
                 << endl;
            if ( from.count() > 0 )
                cout << "    Connected From: " << subsNames(from) << endl;
            if ( to.count() > 0 )
                cout << "    Connecting To: " << subsNames(to) << endl;
        }
    }
}

void systemInfo()
{
    MidiClient* client = new MidiClient();
    client->open();
    client->setClientName(PGM_NAME);
    SystemInfo info = client->getSystemInfo();
    cout << PGM_DESCRIPTION << ", version: "<< QStringLiteral(QT_STRINGIFY(VERSION)) << endl;
    cout << "Compiled ALSA library: " << getCompiledALSALibraryVersion() << endl;
    cout << "Runtime ALSA library: "
         << getRuntimeALSALibraryVersion() << endl;
    cout << "Runtime ALSA drivers: "
         << getRuntimeALSADriverVersion() << endl;
    cout << "Numeric ALSA compiled library: "
         << hex << SND_LIB_VERSION << endl;
    cout << "Numeric ALSA runtime library: "
         << getRuntimeALSALibraryNumber() << endl;
    cout << "Numeric ALSA runtime driver: "
         << getRuntimeALSADriverNumber() << endl;
    cout << "Max Clients: " << dec << info.getMaxClients() << endl;
    cout << "Max Ports: " << info.getMaxPorts() << endl;
    cout << "Max Queues: " << info.getMaxQueues() << endl;
    cout << "Max Channels: " << info.getMaxChannels() << endl;
    cout << "Current Queues: " << info.getCurrentQueues() << endl;
    cout << "Current Clients: " << info.getCurrentClients() << endl;
    queryTimers();
    if (info.getCurrentQueues() > 0)
        queryQueues(client);
    if (info.getCurrentClients() > 0)
        queryClients(client);
    delete client;
}

int main(int argc, char **argv)
{
    const QString ERRORSTR = QStringLiteral("Fatal error from the ALSA sequencer. "
        "This usually happens when the kernel doesn't have ALSA support, "
        "or the device node (/dev/snd/seq) doesn't exists, "
        "or the kernel module (snd_seq) is not loaded. "
        "Please check your ALSA/MIDI configuration.");

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(PGM_NAME);
    QCoreApplication::setApplicationVersion(QStringLiteral(QT_STRINGIFY(VERSION)));

    QCommandLineParser parser;
    parser.setApplicationDescription(PGM_DESCRIPTION);
    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();
    parser.process(app);

    if (parser.isSet(versionOption) || parser.isSet(helpOption)) {
        return 0;
    }

    try {
        systemInfo();
    } catch (const SequencerError& ex) {
        cerr << ERRORSTR << " Returned error was: " << ex.qstrError() << endl;
    } catch (...) {
        cerr << ERRORSTR << endl;
    }
    return 0;
}
