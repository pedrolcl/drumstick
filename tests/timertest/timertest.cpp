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

#include "timertest.h"
#include <iostream>
#include <iomanip> 

using namespace std;

TimerTest::TimerTest()
{
	m_query = new TimerQuery("hw", 0);
}

TimerTest::~TimerTest()
{
	delete m_query;
}

void
TimerTest::run()
{
	TimerIdList ids = m_query->getTimers();
	TimerIdList::iterator it;
	cout << "type__ Name________________ c/s/C/D/S Freq." << endl;
	foreach(TimerId id, ids)
	{
		Timer* timer = new Timer(&id, SND_TIMER_OPEN_NONBLOCK);
		TimerInfo* info = timer->getTimerInfo();
		
		cout << setw(7) << left << info->getId().toStdString();
		cout << setw(20) << left << info->getName().toStdString() << " ";
		cout << id.getClass() << "/" << id.getSlaveClass() << "/";
		cout << id.getCard() << "/" << id.getDevice() << "/" << id.getSubdevice() << " ";
		if( info->isSlave() ) {
			cout << "SLAVE";
		} else {
			long hz = 1000000000 / info->getResolution();
			cout << hz << " Hz"; 
		}
		cout << endl;
		delete info;
		delete timer;
	}
}

int main(int argc, char **argv)
{
  QApplication app(argc, argv, false);
  TimerTest test;
  test.run();
  return 0;
}
