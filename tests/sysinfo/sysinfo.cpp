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

#include <QTextStream>
#include "client.h"

static QTextStream cout(stdout, QIODevice::WriteOnly); 

using namespace ALSA::Sequencer;

void runTest()
{
    MidiClient* client = new MidiClient();
    client->open();
    SystemInfo info = client->getSystemInfo();
    cout << "ALSA Sequencer System Info" << endl;
    cout << "Max Clients: " << info.getMaxClients() << endl;
    cout << "Max Ports: " << info.getMaxPorts() << endl;
    cout << "Max Queues: " << info.getMaxQueues() << endl;
    cout << "Max Channels: " << info.getMaxChannels() << endl;
    cout << "Current Queues: " << info.getCurrentQueues() << endl;
    cout << "Current Clients: " << info.getCurrentClients() << endl;
    delete client;
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv, false);
    runTest();
    return 0;
}
