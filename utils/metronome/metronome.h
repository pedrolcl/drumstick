/*
    Standard MIDI simple metronome
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

#ifndef METRONOME_H
#define METRONOME_H

#include "alsaevent.h"
#include "alsaclient.h"
#include "alsaqueue.h"
#include "alsaport.h"

#include <QObject>
#include <QString>
#include <QList>
#include <QReadWriteLock>

const int TEMPO_DEFAULT(120);
const int NOTE_DURATION(10);
const int RHYTHM_TS_NUM(4);
const int RHYTHM_TS_DEN(4);
const int METRONOME_CHANNEL(9);
const int METRONOME_STRONG_NOTE(34);
const int METRONOME_WEAK_NOTE(33);
const int METRONOME_VELOCITY(100);
const int METRONOME_PROGRAM(0);
const int METRONOME_RESOLUTION(240);
const int METRONOME_VOLUME(100);
const int METRONOME_PAN(64);

using namespace drumstick;

class Metronome : public QObject, public SequencerEventHandler
{
    Q_OBJECT

public:
    Metronome(QObject *parent = 0);
    virtual ~Metronome();
    void play(QString tempo);
    bool stopped();
    void stop();
    void subscribe(const QString& portName);
    void shutupSound();
    //void usage();
    //void info();

    void sendControlChange( int cc, int value );
    void sendInitialControls();
    void metronome_note(int note, int tick);
    void metronome_echo(int tick);
    void metronome_pattern(int tick);
    void metronome_event_output(SequencerEvent* ev);
    void metronome_schedule_event(SequencerEvent* ev, int tick, bool lb);
    void metronome_set_program();
    void metronome_set_tempo();
    void metronome_set_controls();

    // SequencerEventHandler interface
    void handleSequencerEvent( SequencerEvent* ev );

private:
    int m_weak_note;
    int m_strong_note;
    int m_weak_velocity;
    int m_strong_velocity;
    int m_program;
    int m_channel;
    int m_volume;
    int m_pan;
    int m_resolution;
    int m_bpm;
    int m_ts_num; /* time signature: numerator */
    int m_ts_div; /* time signature: denominator */
    int m_noteDuration;
    int m_patternDuration;

    int m_portId;
    int m_queueId;
    int m_clientId;
    bool m_Stopped;
    QReadWriteLock m_mutex;
    MidiClient* m_Client;
    MidiPort* m_Port;
    MidiQueue* m_Queue;
};

#endif /*METRONOME_H*/
