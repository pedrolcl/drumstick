/*
    MIDI Sequencer C++ library 
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

#include "timertest.h"
#include "alsatimer.h"
#include "cmdlineargs.h"
#include <QTextStream>

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
    cout << endl << "type___ Name________________ c/s/C/D/S Freq." << endl;
    TimerIdList lst = query->getTimers();
    TimerIdList::ConstIterator it;
    for( it = lst.constBegin(); it != lst.constEnd(); ++it )
    {
        TimerId id = *it;
        Timer* timer = new Timer(id, SND_TIMER_OPEN_NONBLOCK);
        TimerInfo info = timer->getTimerInfo();
        cout << qSetFieldWidth(8) << left << info.getId();
        cout << qSetFieldWidth(20) << left << info.getName();
        cout << qSetFieldWidth(0) << " ";
        cout << id.getClass() << "/" << id.getSlaveClass() << "/";
        cout << id.getCard() << "/" << id.getDevice() << "/" << id.getSubdevice() << " ";
        if( info.isSlave() ) {
            cout << "SLAVE";
        } else {
            cout << info.getFrequency() << " Hz";
        }
        cout << endl;
        delete timer;
    }
    delete query;
}

void TimerTest::runTest()
{
    QPointer<Timer> test_timer;
    cout << "Looking for the best global ALSA timer ..." << endl;
    test_timer = Timer::bestGlobalTimer( SND_TIMER_OPEN_NONBLOCK | 
                                         SND_TIMER_OPEN_TREAD );
    if (test_timer != NULL) {
        m_info = test_timer->getTimerInfo();
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
            test_timer->setTimerParams(m_params);
            m_status = test_timer->getTimerStatus();
            showStatus();
            test_timer->setHandler(this);
            cout << endl << "Testing timer callback method:" << endl;
            test_timer->start();
            test_timer->startEvents();
            sleep(1);
            test_timer->stopEvents();
            test_timer->stop();
            m_status = test_timer->getTimerStatus();
            showStatus();
            cout << endl << "Success!" << endl;
        } catch (...) {
            cout << endl << "Test failed" << endl;
        }
        delete test_timer;
    }
}

int main(int argc, char **argv)
{
    TimerTest test;
    CmdLineArgs args;
    args.parse(argc, argv);
    test.runTest();
    return 0;
}
