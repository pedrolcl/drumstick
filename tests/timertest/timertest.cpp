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

void read_loop(Timer* timer, int master_ticks, int timeout)
{
    int count, err;
    struct pollfd *fds;
    snd_timer_read_t tr;
    
    count = timer->getPollDescriptorsCount();
    fds = (pollfd *) calloc(count, sizeof(struct pollfd));
    if (fds == NULL) {
        cout << "alloc error" << endl;
        exit(EXIT_FAILURE);
    }
    while (master_ticks-- > 0) {
        timer->pollDescriptors(fds, count);
        if ((err = poll(fds, count, timeout)) < 0) {
            cout << "poll error " << err << "(" << strerror(err) << ")" << endl;
            exit(EXIT_FAILURE);
        }
        if (err == 0) {
            cout << "timer time out!!" << endl;
            exit(EXIT_FAILURE);
        }
        while ( timer->read(&tr, sizeof(tr)) == sizeof(tr) ) {
            cout << "Timer Poll -" 
                 << " resolution = " << tr.resolution << "ns" 
                 << " ticks = " << tr.ticks << endl;
        }
    }
    free(fds);
}

static void async_callback(snd_async_handler_t *ahandler)
{
    snd_timer_read_t tr;
    snd_timer_t *handle = snd_async_handler_get_timer(ahandler);
    int *acount = (int*) snd_async_handler_get_callback_private(ahandler);

    while (snd_timer_read(handle, &tr, sizeof(tr)) == sizeof(tr)) {
        cout << "Timer Callback -" 
             << " resolution = " << tr.resolution << "ns" 
             << " ticks = " << tr.ticks << endl;
    }
    (*acount)++;
}

void showStatus(TimerStatus& status)
{
    cout << endl << "STATUS:" << endl;
    cout << "  resolution = " << status.getResolution() << endl;
    cout << "  lost = " << status.getLost() << endl;
    cout << "  overrun = " << status.getOverrun() << endl;
    cout << "  queue = " << status.getQueue() << endl;
}

void showInfo(TimerInfo& info)
{
    cout << endl << "Timer info:" << endl;
    cout << "  slave = " << (info.isSlave() ? "Yes" : "No") << endl;
    cout << "  card = " << info.getCard() << endl;
    cout << "  id = " << info.getId() << endl;
    cout << "  name = " << info.getName() << endl;
    cout << "  average resolution = " << info.getResolution() << endl;
}

void runTest()
{
    TimerQuery* query = new TimerQuery("hw", 0);
    cout << "Here is a listing of your available ALSA timers..." << endl;
    cout << "type__ Name________________ c/s/C/D/S Freq." << endl;
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
            cout << info.getFrequency() << " Hz"; 
        }
        cout << endl;
        delete timer;
    }
    delete query;
    
    cout << endl << "Testing now the RTC timer" << endl;
    try {
        Timer* timer = new Timer(1, 0, 0, 1, 0, SND_TIMER_OPEN_NONBLOCK);
        TimerInfo info = timer->getTimerInfo();
        TimerParams params;
        showInfo(info);
        
        params.setAutoStart(true);
        if (!info.isSlave()) {
            params.setTicks(1000000000 / info.getResolution() / 50); /* 50Hz */
            if (params.getTicks() < 1) {
                params.setTicks(1);
            }
            cout << "Using " << params.getTicks() << " tick(s)" << endl;
        } else {
            params.setTicks(1);            
        }
        timer->setTimerParams(params);
        showStatus(timer->getTimerStatus());

        cout << endl << "Testing timer poll method:" << endl;
        timer->start();
        read_loop(timer, 25, info.isSlave() ? 10000 : 25);
        timer->stop();
        showStatus(timer->getTimerStatus());
        
        cout << endl << "Testing timer callback method:" << endl;
        int acount = 0;
        timer->addAsyncTimerHandler(async_callback, &acount);
        timer->start();
        while (acount < 25)
            sleep(1);
        timer->stop();
        showStatus(timer->getTimerStatus());
        
        cout << endl << "Success!" << endl;
        delete timer;
    } catch (...) {
        cout << endl << "Test failed" << endl;
    }
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv, false);
    runTest();
    return 0;
}
