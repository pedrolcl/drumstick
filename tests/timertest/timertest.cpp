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
#include "alsatimer.h"
#include "timertest.h"

static QTextStream cout(stdout, QIODevice::WriteOnly); 

void TimerTest::handleTimerEvent(int ticks, int msecs)
{
    cout << "Timer callback msecs = " << msecs
         <<               " ticks = " << ticks
         << endl;
}

void TimerTest::showStatus()
{
    cout << endl << "STATUS:" << endl;
    cout << "  resolution = " << m_status.getResolution() << endl;
    cout << "  lost = " << m_status.getLost() << endl;
    cout << "  overrun = " << m_status.getOverrun() << endl;
    cout << "  queue = " << m_status.getQueue() << endl;
}

void TimerTest::showInfo()
{
    cout << endl << "Timer info:" << endl;
    cout << "  slave = " << (m_info.isSlave() ? "Yes" : "No") << endl;
    cout << "  card = " << m_info.getCard() << endl;
    cout << "  id = " << m_info.getId() << endl;
    cout << "  name = " << m_info.getName() << endl;
    cout << "  average resolution = " << m_info.getResolution() << endl;
    cout << "  frequency = " << m_info.getFrequency() << " Hz" << endl;
}

void TimerTest::queryTimers()
{
    TimerQuery* query = new TimerQuery("hw", 0);
    cout << endl << "type__ Name________________ c/s/C/D/S Freq." << endl;
    TimerIdList lst = query->getTimers();
    TimerIdList::ConstIterator it;
    for( it = lst.begin(); it != lst.end(); ++it )
    {
        TimerId id = *it;
        Timer* timer = new Timer(id, SND_TIMER_OPEN_NONBLOCK);
        TimerInfo info = timer->getTimerInfo();
        cout << qSetFieldWidth(7) << left << info.getId();
        cout << qSetFieldWidth(20) << left << info.getName();
        cout << qSetFieldWidth(0) << " ";
        cout << id.getClass() << "/" << id.getSlaveClass() << "/";
        cout << id.getCard() << "/" << id.getDevice() << "/" << id.getSubdevice() << " ";
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

void TimerTest::runTest()
{
    cout << "Looking for the best global ALSA timer ..." << endl;
    m_timer = Timer::bestGlobalTimer( SND_TIMER_OPEN_NONBLOCK | 
                                      SND_TIMER_OPEN_TREAD );
    m_info = m_timer->getTimerInfo();
    showInfo();
    cout << endl << "Here is a listing of all your available ALSA timers ..." << endl;
    queryTimers();
    cout << endl << "Testing the best available timer ..." << endl;
    try {
        m_params.setAutoStart(true);
        if (!m_info.isSlave()) {
            /* 50 Hz */
            m_params.setTicks( 1000000000L / m_info.getResolution() / 50); 
            if (m_params.getTicks() < 1) {
                m_params.setTicks(1);
            }
            cout << "Using " << m_params.getTicks() << " tick(s)" << endl;
        } else {
            m_params.setTicks(1);            
        }
        m_params.setFilter(1 << SND_TIMER_EVENT_TICK);
        m_timer->setTimerParams(m_params);
        m_status = m_timer->getTimerStatus();
        showStatus();
        m_timer->setHandler(this);
        cout << endl << "Testing timer callback method:" << endl;
        m_timer->start();
        m_timer->startEvents();
        sleep(1);
        m_timer->stopEvents();
        m_timer->stop();
        m_status = m_timer->getTimerStatus();
        showStatus();
        cout << endl << "Success!" << endl;
        delete m_timer;
    } catch (...) {
        cout << endl << "Test failed" << endl;
    }
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv, false);
    TimerTest test;
    test.runTest();
    return 0;
}
