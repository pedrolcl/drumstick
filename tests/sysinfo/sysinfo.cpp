/*
    MIDI Sequencer C++ library 
    Copyright (C) 2006-2009, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QTextStream>
#include <QStringList>
#include <QFileInfo>

#include "alsatimer.h"
#include "alsaqueue.h"
#include "alsaclient.h"
#include "subscription.h"

static QTextStream cout(stdout, QIODevice::WriteOnly); 

using namespace aseqmm;

void queryTimers()
{
    cout << endl << "ALSA Timers" << endl;
    TimerQuery* query = new TimerQuery("hw", 0);
    cout << "type__ Name________________ c/s/C/D/S Freq." << endl;
    TimerIdList lst = query->getTimers();
    TimerIdList::ConstIterator it;
    for( it = lst.begin(); it != lst.end(); ++it )
    {
        TimerId id = *it;
        Timer* timer = new Timer(id, SND_TIMER_OPEN_NONBLOCK);
        TimerInfo info = timer->getTimerInfo();
        cout << qSetFieldWidth(7) << left << info.getId() 
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
        if (queue != NULL) {
            QueueInfo qinfo = queue->getInfo();
            QueueStatus qsts = queue->getStatus();
            QueueTempo qtmp = queue->getTempo();
            QueueTimer qtmr = queue->getTimer();
            TimerId tid(qtmr.getId());
            Timer* timer = new Timer(tid, SND_TIMER_OPEN_NONBLOCK);
            TimerInfo tinfo = timer->getTimerInfo();
            cout << qSetFieldWidth(3)  << left << qinfo.getId()
                 << qSetFieldWidth(20) << qinfo.getName().left(20)
                 << qSetFieldWidth(0)  << " "
                 << qSetFieldWidth(20) << tinfo.getName().left(20)
                 << qSetFieldWidth(6)  << right << qinfo.getOwner()
                 << qSetFieldWidth(7)  << (qinfo.isLocked() ? "locked" : "free")
                 << qSetFieldWidth(8)  << (qsts.isRunning() ? "running" : "stopped")
                 << qSetFieldWidth(4)  << qtmp.getPPQ()
                 << qSetFieldWidth(7)  << qtmp.getRealBPM()
                 << qSetFieldWidth(4)  << qtmp.getNominalBPM()
                 << qSetFieldWidth(8)  << qsts.getTickTime()
                 << qSetFieldWidth(0)  << " " << qsts.getClockTime()
                 << endl;
            delete timer;
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
        QString sname = QString("%1:%2").arg((int)s.getAddr()->client)
                                        .arg((int)s.getAddr()->port);
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
        cout << "Client" << qSetFieldWidth(4) << cinfo.getClientId()
             << qSetFieldWidth(0) << " : \"" << cinfo.getName() << "\" ["
             << clientTypeName(cinfo.getClientType())
             << "]" << endl;
        foreach( PortInfo pinfo, plist ) {
            SubscribersList to = pinfo.getReadSubscribers();
            SubscribersList from = pinfo.getWriteSubscribers();
            cout << "  Port" << qSetFieldWidth(4) << pinfo.getPort()
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
    QFileInfo pgmi(qApp->arguments().at(0));
    client->open();
    client->setClientName(pgmi.baseName());
    SystemInfo info = client->getSystemInfo();
    cout << "ALSA Sequencer System Info" << endl;
    cout << "Max Clients: " << info.getMaxClients() << endl;
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
    QApplication app(argc, argv, false);
    systemInfo();
    return 0;
}
