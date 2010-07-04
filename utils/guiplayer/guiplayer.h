/*
    SMF GUI Player test using the MIDI Sequencer C++ library
    Copyright (C) 2006-2010, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include "ui_guiplayer.h"
#include "song.h"

#include <QtGui/QWidget>
#include <QtGui/QProgressDialog>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QPointer>

namespace drumstick {
    class QSmf;
    class QWrk;
    class MidiClient;
    class MidiPort;
    class MidiQueue;
}

class Player;

using namespace drumstick;

const QString QSTR_DOMAIN("drumstick.sourceforge.net");
const QString QSTR_APPNAME("GUIPlayer");

class GUIPlayer : public QWidget
{
    Q_OBJECT

public:
    GUIPlayer(QWidget *parent = 0);
    ~GUIPlayer();

    void appendSMFEvent(SequencerEvent* ev);
    void appendWRKEvent(unsigned long ticks, int track, SequencerEvent* ev);

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

public slots:
    void play();
    void pause();
    void stop();
    void open();
    void setup();
    void tempoReset();
    void tempoSlider(int value);
    void quit();

    void songFinished();
    void playerStopped();
    void sequencerEvent(SequencerEvent* ev);

    /* SMF slots */
    void headerEvent(int format, int ntrks, int division);
    void noteOnEvent(int chan, int pitch, int vol);
    void noteOffEvent(int chan, int pitch, int vol);
    void keyPressEvent(int chan, int pitch, int press);
    void ctlChangeEvent(int chan, int ctl, int value);
    void pitchBendEvent(int chan, int value);
    void programEvent(int chan, int patch);
    void chanPressEvent(int chan, int press);
    void sysexEvent(const QByteArray& data);
    void textEvent(int type, const QString& data);
    void tempoEvent(int tempo);
    void errorHandler(const QString& errorStr);
    void updateSMFLoadProgress();

    /* WRK slots */
    void updateWRKLoadProgress();
    void errorHandlerWRK(const QString& errorStr);
    void unknownChunk(int type, const QByteArray& data);
    void fileHeader(int verh, int verl);
    void endOfWrk();
    void streamEndEvent(long time);
    void trackHeader(const QString& name1, const QString& name2,
                     int trackno, int channel, int pitch,
                     int velocity, int port,
                     bool selected, bool muted, bool loop);
    void timeBase(int timebase);
    void globalVars();
    void noteEvent(int track, long time, int chan, int pitch, int vol, int dur);
    void keyPressEvent(int track, long time, int chan, int pitch, int press);
    void ctlChangeEvent(int track, long time, int chan, int ctl, int value);
    void pitchBendEvent(int track, long time, int chan, int value);
    void programEvent(int track, long time, int chan, int patch);
    void chanPressEvent(int track, long time, int chan, int press);
    void sysexEvent(int track, long time, int bank);
    void sysexEventBank(int bank, const QString& name, bool autosend, int port, const QByteArray& data);
    void textEvent(int track, long time, int typ, const QString& data);
    void timeSigEvent(int bar, int num, int den);
    void keySigEventWRK(int bar, int alt);
    void tempoEvent(long time, int tempo);
    void trackPatch(int track, int patch);
    void comments(const QString& cmt);
    void variableRecord(const QString& name, const QByteArray& data);
    void newTrackHeader(const QString& name,
                        int trackno, int channel, int pitch,
                        int velocity, int port,
                        bool selected, bool muted, bool loop);
    void trackName(int trackno, const QString& name);
    void trackVol(int track, int vol);
    void trackBank(int track, int bank);
    void segment(int track, long time, const QString& name);
    void chord(int track, long time, const QString& name, const QByteArray& data);
    void expression(int track, long time, int code, const QString& text);

private:
    int m_portId;
    int m_queueId;
    int m_initialTempo;
    float m_tempoFactor;
    unsigned long m_tick;

    QSmf* m_smf;
    QWrk* m_wrk;
    MidiClient* m_Client;
    MidiPort* m_Port;
    MidiQueue* m_Queue;
    Player* m_player;

    QString m_subscription;
    QString m_lastDirectory;
    QString m_loadingMessages;
    Song m_song;
    Ui::GUIPlayerClass ui;
    QPointer<QProgressDialog> m_pd;

    struct SysexEventRec {
        int track;
        long time;
        int bank;
    };
    QList<SysexEventRec> m_savedSysexEvents;
    QString m_fileFormat;

    struct TrackMapRec {
        int channel;
        int pitch;
        int velocity;
    };
    QHash<int,TrackMapRec> m_trackMap;

    struct TimeSigRec {
        int bar;
        int num;
        int den;
        long time;
    };
    QList<TimeSigRec> m_bars;

};

#endif // INCLUDED_GUIPLAYER_H
