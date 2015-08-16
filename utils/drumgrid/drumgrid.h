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

#ifndef DRUMGRID_H
#define DRUMGRID_H

#include "drumgridabout.h"

#include <QMainWindow>
#include <QShortcut>
#include <QCloseEvent>
#include <QSignalMapper>

const QString QSTR_WINDOW("Window");
const QString QSTR_GEOMETRY("Geometry");
const QString QSTR_STATE("State");
const QString QSTR_MIDI("MIDI");
const QString QSTR_CONNECTION("Connection");
const QString QSTR_TEMPO("Tempo");
const QString QSTR_PATTERN("Pattern");

const int TEMPO_MIN(25);
const int TEMPO_MAX(250);
const int TEMPO_DEFAULT(120);
const int NOTE_DURATION(10);
const int METRONOME_CHANNEL(9);
const int METRONOME_VELOCITY(100);
const int METRONOME_PROGRAM(0);
const int METRONOME_RESOLUTION(120);
const int METRONOME_VOLUME(100);
const int METRONOME_PAN(64);
const int VOLUME_CC(7);
const int PAN_CC(10);

namespace Ui
{
    class DrumGrid;
}

namespace drumstick
{
    class MidiClient;
    class MidiPort;
    class MidiQueue;
    class SequencerEvent;
}

class DrumGridModel;

using namespace drumstick;

class DrumGrid : public QMainWindow
{
    Q_OBJECT

public:
    DrumGrid(QWidget *parent = 0);
    ~DrumGrid();
    void subscribe(const QString& portName);
    void addShortcut(const QKeySequence& key, const QString& value);
    void readSettings();
    void writeSettings();
    void closeEvent( QCloseEvent *event );

    void metronome_start();
    void metronome_stop();
    void metronome_continue();

    void sendControlChange(int cc, int value);
    void sendInitialControls();
    void metronome_set_controls();
    void metronome_set_program();
    void metronome_set_tempo();

    void metronome_pattern(int tick);
    void metronome_echo(int tick, int ev_type);
    void metronome_note(int note, int vel, int tick);
    void metronome_schedule_event(SequencerEvent* ev, int tick);
    void metronome_event_output(SequencerEvent* ev);
    int decodeVelocity(const QString drumVel);

public slots:
    void slotAbout();
    void slotAboutQt();
    void updateView();
    void sequencerEvent(SequencerEvent *ev);
    void connectMidi();
    void play();
    void stop();
    void tempoChange(int newTempo);
    void gridColumns(int columns);
    void shortcutPressed(const QString& value);
    void updateDisplay(int bar, int beat);

signals:
    void signalUpdate(int bar, int beat);

private:
    Ui::DrumGrid *m_ui;
    int m_clientId;
    int m_portId;
    int m_queueId;
    unsigned long m_tick;
    MidiClient* m_Client;
    MidiPort* m_Port;
    MidiQueue* m_Queue;
    DrumGridModel* m_model;
    QString m_subscription;
    QSignalMapper* m_mapper;
    QVector<QShortcut*> m_shortcuts;
    About dlgAbout;

    int m_bar;
    int m_beat;
    int m_weak_velocity;
    int m_strong_velocity;
    int m_program;
    int m_channel;
    int m_volume;
    int m_pan;
    int m_resolution;
    int m_bpm;
    int m_noteDuration;
    int m_patternDuration;
    bool m_autoconnect;
    bool m_playing;
    bool m_useNoteOff;
};

#endif // DRUMGRID_H
