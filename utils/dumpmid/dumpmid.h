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

#ifndef DUMPMIDI_H_
#define DUMPMIDI_H_

/* MidiClient can deliver SequencerEvents with only
 * signals or posting QEvents to the QApplication loop */
#undef USE_QEVENTS
//#define USE_QEVENTS

/* Tp get timestamped events from ALSA, you need a running queue */
//#undef WANT_TIMESTAMPS
#define WANT_TIMESTAMPS

#include <QObject>
#include <QReadWriteLock>

#include "alsaevent.h"
#include "alsaclient.h"
#include "alsaport.h"
#include "alsaqueue.h"
#include "subscription.h"

using namespace drumstick;

class QDumpMIDI : public QObject
{
    Q_OBJECT

public:
    QDumpMIDI();
    virtual ~QDumpMIDI();

    void dumpEvent(SequencerEvent* ev);
    void subscribe(const QString& portName);
    void stop();
    bool stopped();
    void run();

public slots:
    void subscription(MidiPort* port, Subscription* subs);

#ifdef USE_QEVENTS
protected:
    virtual void customEvent( QEvent *ev );
#else
    void sequencerEvent( SequencerEvent* ev );
#endif

private:
    MidiClient* m_Client;
    MidiPort* m_Port;
#ifdef WANT_TIMESTAMPS
    MidiQueue* m_Queue;
#endif
    bool m_Stopped;
    QReadWriteLock m_mutex;
};

#endif /*DUMPMIDI_H_*/
