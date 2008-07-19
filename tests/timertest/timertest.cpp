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

#include "alsatimer.h"
#include <QTextStream>

static QTextStream cout(stdout, QIODevice::WriteOnly); 

using namespace ALSA;

void runTest()
{
    TimerQuery* query = new TimerQuery("hw", 0);
    cout << "type__ Name________________ c/s/C/D/S Freq." << endl;
    TimerIdList lst = query->getTimers();
    TimerIdList::ConstIterator it;
    for( it = lst.begin(); it != lst.end(); ++it )
    {
        TimerId id = *it;
        Timer* timer = new Timer(&id, SND_TIMER_OPEN_NONBLOCK);
        TimerInfo* info = timer->getTimerInfo();
        cout << qSetFieldWidth(7) << left << info->getId();
        cout << qSetFieldWidth(20) << left << info->getName();
        cout << qSetFieldWidth(0) << " ";
        cout << id.getClass() << "/" << id.getSlaveClass() << "/";
        cout << id.getCard() << "/" << id.getDevice() << "/" << id.getSubdevice() << " ";
        if( info->isSlave() ) {
            cout << "SLAVE";
        } else {
            cout << info->getFrequency() << " Hz"; 
        }
        cout << endl;
        delete info;
        delete timer;
    }
    delete query;
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv, false);
    runTest();
    return 0;
}
