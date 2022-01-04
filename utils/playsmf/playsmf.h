/*
    Standard MIDI File player program 
    Copyright (C) 2006-2022, Pedro Lopez-Cabanillas <plcl@users.sf.net>
 
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

#ifndef PLAYSMF_H_
#define PLAYSMF_H_

#include <QObject>
#include <QString>
#include <QList>
#include <QReadWriteLock>

#include <drumstick/qsmf.h>
#include <drumstick/alsaevent.h>
#include <drumstick/alsaclient.h>
#include <drumstick/alsaqueue.h>
#include <drumstick/alsaport.h>

class Song : public QList<drumstick::ALSA::SequencerEvent*>
{
public:
    virtual ~Song();
    void sort();
    void clear();
};

class PlaySMF : public QObject
{
    Q_OBJECT

public:
    PlaySMF();
    virtual ~PlaySMF();
    void play(QString fileName);
    bool stopped();
    void stop();
    void appendEvent(drumstick::ALSA::SequencerEvent* ev);
    void subscribe(const QString& portName);
    void dump(const QString& chan, const QString& event, const QString& data);
    void dumpStr(const QString& event, const QString& data);
    void shutupSound();

public slots:
    void headerEvent(int format, int ntrks, int division);
    void noteOnEvent(int chan, int pitch, int vol);
    void noteOffEvent(int chan, int pitch, int vol);
    void keyPressEvent(int chan, int pitch, int press);
    void ctlChangeEvent(int chan, int ctl, int value);
    void pitchBendEvent(int chan, int value);
    void programEvent(int chan, int patch);
    void chanPressEvent(int chan, int press);
    void sysexEvent(const QByteArray& data);
    void textEvent(int typ, const QString& data);
    void tempoEvent(int tempo);
    void timeSigEvent(int b0, int b1, int b2, int b3);
    void keySigEvent(int b0, int b1);
    void errorHandler(const QString& errorStr);

private:
    int m_division;
    int m_portId;
    int m_queueId;
    int m_initialTempo;
    bool m_Stopped;
    QReadWriteLock m_mutex;
    Song m_song;
    drumstick::File::QSmf* m_engine;
    drumstick::ALSA::MidiClient* m_Client;
    drumstick::ALSA::MidiPort* m_Port;
    drumstick::ALSA::MidiQueue* m_Queue;
};

#endif /*PLAYSMF_H_*/
