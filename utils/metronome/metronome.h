/*
    Standard MIDI simple metronome
    Copyright (C) 2006-2020, Pedro Lopez-Cabanillas <plcl@users.sf.net>
 
    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef METRONOME_H
#define METRONOME_H

#include <QObject>
#include <QString>
#include <QList>
#include <QReadWriteLock>

#include <drumstick/alsaevent.h>
#include <drumstick/alsaclient.h>
#include <drumstick/alsaqueue.h>
#include <drumstick/alsaport.h>

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

class Metronome : public QObject, public drumstick::ALSA::SequencerEventHandler
{
    Q_OBJECT

public:
    explicit Metronome(QObject *parent = nullptr);
    virtual ~Metronome();
    void play(QString tempo);
    bool stopped();
    void stop();
    void subscribe(const QString& portName);
    void shutupSound();

    void sendControlChange( int cc, int value );
    void sendInitialControls();
    void metronome_note(int note, int tick);
    void metronome_echo(int tick);
    void metronome_pattern(int tick);
    void metronome_event_output(drumstick::ALSA::SequencerEvent* ev);
    void metronome_schedule_event(drumstick::ALSA::SequencerEvent* ev, int tick, bool lb);
    void metronome_set_program();
    void metronome_set_tempo();
    void metronome_set_controls();

    // SequencerEventHandler interface
    void handleSequencerEvent( drumstick::ALSA::SequencerEvent* ev ) override;

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
    drumstick::ALSA::MidiClient* m_Client;
    drumstick::ALSA::MidiPort* m_Port;
    drumstick::ALSA::MidiQueue* m_Queue;
};

#endif /*METRONOME_H*/
