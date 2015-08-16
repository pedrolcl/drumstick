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

#ifndef TESTEVENTS_H_
#define TESTEVENTS_H_

#include "alsaevent.h"
#include <QObject>

using namespace drumstick;

class QTestEvents : public QObject
{
    Q_OBJECT

public:
    QTestEvents() {}
    virtual ~QTestEvents() {}

    void dumpEvent(SequencerEvent* m_ev);
    void run();
};

#endif /*TESTEVENTS_H_*/
