/*
    SMF GUI Player test using the MIDI Sequencer C++ library
    Copyright (C) 2006-2015, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef INCLUDED_GUIPLAYER_H
#define INCLUDED_GUIPLAYER_H

#include <QMainWindow>
#include <QProgressDialog>
#include <QObject>
#include <QString>
#include <QList>
#include <QHash>
#include <QPointer>

namespace drumstick {
    class QSmf;
    class QWrk;
    class QOve;
    class MidiClient;
    class MidiPort;
    class MidiQueue;
    class SequencerEvent;
}

namespace Ui {
    class GUIPlayerClass;
}

class Player;
class About;
class Song;

using namespace drumstick;

const QString QSTR_DOMAIN("drumstick.sourceforge.net");
const QString QSTR_APPNAME("GUIPlayer");

enum PlayerState {
    InvalidState,
    EmptyState,
    PlayingState,
    PausedState,
    StoppedState
};

class GUIPlayer : public QMainWindow
{
    Q_OBJECT

public:
    GUIPlayer(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~GUIPlayer();

    void appendSMFEvent(SequencerEvent* ev);
    void appendWRKEvent(unsigned long ticks, SequencerEvent* ev);
    void appendOVEEvent(unsigned long ticks, SequencerEvent* ev);

    void subscribe(const QString& portName);
    void updateTimeLabel(int mins, int secs, int cnts);
    void updateTempoLabel(float ftempo);
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);
    void closeEvent(QCloseEvent* event);
    bool event(QEvent* event);
    void openFile(const QString& fileName);
    void readSettings();
    void writeSettings();
    void updateState(PlayerState newState);

    void progressDialogInit(const QString& type, int max);
    void progressDialogUpdate(int pos);
    void progressDialogClose();

public slots:
    void about();
    void aboutQt();
    void play();
    void pause();
    void stop();
    void open();
    void setup();
    void tempoReset();
    void volumeReset();
    void tempoSlider(int value);
    void quit();
    void volumeSlider(int value);
    void pitchShift(int value);
    void songFinished();
    void playerStopped();
    void sequencerEvent(SequencerEvent* ev);

    /* SMF slots */
    void smfHeaderEvent(int format, int ntrks, int division);
    void smfNoteOnEvent(int chan, int pitch, int vol);
    void smfNoteOffEvent(int chan, int pitch, int vol);
    void smfKeyPressEvent(int chan, int pitch, int press);
    void smfCtlChangeEvent(int chan, int ctl, int value);
    void smfPitchBendEvent(int chan, int value);
    void smfProgramEvent(int chan, int patch);
    void smfChanPressEvent(int chan, int press);
    void smfSysexEvent(const QByteArray& data);
    void smfTempoEvent(int tempo);
    void smfErrorHandler(const QString& errorStr);
    void smfUpdateLoadProgress();

    /* WRK slots */
    void wrkUpdateLoadProgress();
    void wrkErrorHandler(const QString& errorStr);
    void wrkFileHeader(int verh, int verl);
    void wrkEndOfFile();
    void wrkStreamEndEvent(long time);
    void wrkTrackHeader(const QString& name1, const QString& name2,
                     int trackno, int channel, int pitch,
                     int velocity, int port,
                     bool selected, bool muted, bool loop);
    void wrkTimeBase(int timebase);
    void wrkNoteEvent(int track, long time, int chan, int pitch, int vol, int dur);
    void wrkKeyPressEvent(int track, long time, int chan, int pitch, int press);
    void wrkCtlChangeEvent(int track, long time, int chan, int ctl, int value);
    void wrkPitchBendEvent(int track, long time, int chan, int value);
    void wrkProgramEvent(int track, long time, int chan, int patch);
    void wrkChanPressEvent(int track, long time, int chan, int press);
    void wrkSysexEvent(int track, long time, int bank);
    void wrkSysexEventBank(int bank, const QString& name, bool autosend, int port, const QByteArray& data);
    void wrkTempoEvent(long time, int tempo);
    void wrkTrackPatch(int track, int patch);
    void wrkNewTrackHeader(const QString& name,
                        int trackno, int channel, int pitch,
                        int velocity, int port,
                        bool selected, bool muted, bool loop);
    void wrkTrackVol(int track, int vol);
    void wrkTrackBank(int track, int bank);

    /* OVE slots */
    void oveErrorHandler(const QString& errorStr);
    void oveFileHeader(int quarter, int trackCount);
    void oveNoteOnEvent(int track, long tick, int channel, int pitch, int vol);
    void oveNoteOffEvent(int track, long tick, int channel, int pitch, int vol);
    void oveTrackPatch(int track, int channel, int patch);
    void oveTrackVol(int track, int channel, int vol);
    void oveTrackBank(int track, int channel, int bank);

private:
    int m_portId;
    int m_queueId;
    int m_initialTempo;
    float m_tempoFactor;
    unsigned long m_tick;
    PlayerState m_state;

    QSmf* m_smf;
    QWrk* m_wrk;
    QOve* m_ove;
    MidiClient* m_Client;
    MidiPort* m_Port;
    MidiQueue* m_Queue;
    Player* m_player;
    Ui::GUIPlayerClass* m_ui;
    QPointer<QProgressDialog> m_pd;
    QPointer<About> m_aboutDlg;
    Song* m_song;

    QString m_subscription;
    QString m_lastDirectory;
    QString m_loadingMessages;

    struct SysexEventRec {
        int track;
        long time;
        int bank;
    };
    QList<SysexEventRec> m_savedSysexEvents;

    struct TrackMapRec {
        int channel;
        int pitch;
        int velocity;
    };
    QHash<int,TrackMapRec> m_trackMap;
};

#endif // INCLUDED_GUIPLAYER_H
