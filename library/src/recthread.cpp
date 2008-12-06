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

#include "commons.h"
#include "recthread.h"
#include "client.h"

namespace ALSA 
{
namespace Sequencer 
{

bool SequencerInputThread::stopped() 
{ 
    m_mutex.lockForRead();
    bool bTmp = m_Stopped;
    m_mutex.unlock();
    return  bTmp;
}

void SequencerInputThread::stop() 
{ 
    m_mutex.lockForWrite();
    m_Stopped = true;
    m_mutex.unlock();
}

void SequencerInputThread::run()
{
    unsigned long npfd;
    pollfd* pfd;
    int rt;

    if (m_MidiClient != NULL) {
        npfd = snd_seq_poll_descriptors_count(m_MidiClient->getHandle(), POLLIN);
        pfd = (pollfd *) alloca(npfd * sizeof(pollfd));
        try
        {
            snd_seq_poll_descriptors(m_MidiClient->getHandle(), pfd, npfd, POLLIN);
            while (!stopped() && (m_MidiClient != NULL))
            {
                rt = poll(pfd, npfd, m_Wait);
                if (rt > 0) {
                    m_MidiClient->doEvents();
                }
            }
        }
        catch (...)
        {
            qWarning() << "exception in input thread";
        }
    }
}

}
}
