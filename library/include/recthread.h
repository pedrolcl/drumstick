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
 
#ifndef INCLUDED_RECTHREAD_H
#define INCLUDED_RECTHREAD_H

#include "commons.h"
#include <QThread>
#include <QMutex>

namespace ALSA 
{
namespace Sequencer 
{

class MidiClient; 

class SequencerInputThread: public QThread
{
    Q_OBJECT

public:
    SequencerInputThread(MidiClient *seq, int timeout) 
        : QThread(),
        m_MidiClient(seq),
        m_Wait(timeout),
        m_Stopped(false) { }
    virtual void run();
    bool stopped();
    void stop();

private:
    MidiClient *m_MidiClient;
    int m_Wait;
    bool m_Stopped;
    QMutex m_mutex;
};

}
}

#endif //INCLUDED_RECTHREAD_H
