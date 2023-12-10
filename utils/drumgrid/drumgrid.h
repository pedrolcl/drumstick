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

#ifndef DRUMGRID_H
#define DRUMGRID_H

#include <QMainWindow>
#include <QShortcut>
#include <QCloseEvent>

namespace Ui
{
    class DrumGrid;
}

namespace drumstick {
namespace ALSA {
    class MidiClient;
    class MidiPort;
    class MidiQueue;
    class SequencerEvent;
}}

class DrumGridModel;

class DrumGrid : public QMainWindow
{
    Q_OBJECT

public:
    explicit DrumGrid(QWidget *parent = nullptr);
    ~DrumGrid();
    void subscribe(const QString& portName);
    void addShortcut(const QKeySequence& key, const QString& value);
    void readSettings();
    void writeSettings();
    void closeEvent( QCloseEvent *event ) override;

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
    void metronome_schedule_event(drumstick::ALSA::SequencerEvent* ev, int tick);
    void metronome_event_output(drumstick::ALSA::SequencerEvent* ev);
    int decodeVelocity(const QString drumVel);

    static const QString QSTR_WINDOW;
    static const QString QSTR_GEOMETRY;
    static const QString QSTR_STATE;
    static const QString QSTR_MIDI;
    static const QString QSTR_CONNECTION;
    static const QString QSTR_TEMPO;
    static const QString QSTR_PATTERN;

    static const int TEMPO_MIN;
    static const int TEMPO_MAX;
    static const int TEMPO_DEFAULT;
    static const int NOTE_DURATION;
    static const int METRONOME_CHANNEL;
    static const int METRONOME_VELOCITY;
    static const int METRONOME_PROGRAM;
    static const int METRONOME_RESOLUTION;
    static const int METRONOME_VOLUME;
    static const int METRONOME_PAN;
    static const int VOLUME_CC;
    static const int PAN_CC;

public Q_SLOTS:
    void slotAbout();
    void updateView();
    void sequencerEvent(drumstick::ALSA::SequencerEvent *ev);
    void connectMidi();
    void play();
    void stop();
    void tempoChange(int newTempo);
    void gridColumns(int columns);
    void shortcutPressed(const QString& value);
    void updateDisplay(int bar, int beat);

Q_SIGNALS:
    void signalUpdate(int bar, int beat);

private:
    Ui::DrumGrid *m_ui;
    int m_clientId;
    int m_portId;
    int m_queueId;
    unsigned long m_tick;
    drumstick::ALSA::MidiClient* m_Client;
    drumstick::ALSA::MidiPort* m_Port;
    drumstick::ALSA::MidiQueue* m_Queue;
    DrumGridModel* m_model;
    QString m_subscription;
    QVector<QShortcut*> m_shortcuts;

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
