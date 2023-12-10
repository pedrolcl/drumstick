/*
    MIDI Sequencer C++ library
    Copyright (C) 2006-2023, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DUMPMIDI_H_
#define DUMPMIDI_H_

/* MidiClient can deliver SequencerEvents with only
 * signals or posting QEvents to the QApplication loop */
#undef USE_QEVENTS
//#define USE_QEVENTS

/* To get timestamped events from ALSA, you need a running queue */
//#undef WANT_TIMESTAMPS
#define WANT_TIMESTAMPS

#include <QObject>
#include <QReadWriteLock>

#include <drumstick/alsaevent.h>
#include <drumstick/alsaclient.h>
#include <drumstick/alsaport.h>
#include <drumstick/alsaqueue.h>
#include <drumstick/subscription.h>

class QDumpMIDI : public QObject, public drumstick::ALSA::SequencerEventHandler
{
    Q_OBJECT
public:
    QDumpMIDI();
    virtual ~QDumpMIDI();

    void dumpEvent(drumstick::ALSA::SequencerEvent* ev);
    void subscribe(const QString& portName);
    void stop();
    bool stopped();
    void run();

    virtual void handleSequencerEvent(drumstick::ALSA::SequencerEvent* ev);

public Q_SLOTS:
    void subscription( drumstick::ALSA::MidiPort* port, drumstick::ALSA::Subscription* subs );

#ifdef USE_QEVENTS
protected:
    virtual void customEvent( QEvent *ev );
#else
    void sequencerEvent( drumstick::ALSA::SequencerEvent* ev );
#endif

private:
    drumstick::ALSA::MidiClient* m_Client;
    drumstick::ALSA::MidiPort* m_Port;
#ifdef WANT_TIMESTAMPS
    drumstick::ALSA::MidiQueue* m_Queue;
#endif
    bool m_Stopped;
    QReadWriteLock m_mutex;
};

#endif /*DUMPMIDI_H_*/
