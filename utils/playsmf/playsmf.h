/*
    Standard MIDI File player program 
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

#ifndef PLAYSMF_H_
#define PLAYSMF_H_

#include <QObject>
#include <QString>
#include <QList>
#include <QReadWriteLock>

#include "qsmf.h"
#include "alsaevent.h"
#include "alsaclient.h"
#include "alsaqueue.h"
#include "alsaport.h"

using namespace drumstick;

class Song : public QList<SequencerEvent*>
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
    void appendEvent(SequencerEvent* ev);
    void subscribe(const QString& portName);
    void dump(const QString& chan, const QString& event, const QString& data);
    void dumpStr(const QString& event, const QString& data);
    void shutupSound();
    //void usage();
    //void info();

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

//  void trackStartEvent();
//  void trackEndEvent();
//  void endOfTrackEvent();
//  void variableEvent(const QByteArray& data);
//  void metaMiscEvent(int typ, const QByteArray& data);
//  void seqNum(int seq);
//  void forcedChannel(int channel);
//  void forcedPort(int port);
//  void smpteEvent(int b0, int b1, int b2, int b3, int b4);

private:
    int m_division;
    int m_portId;
    int m_queueId;
    int m_initialTempo;
    bool m_Stopped;
    QReadWriteLock m_mutex;
    Song m_song;
    QSmf* m_engine;
    MidiClient* m_Client;
    MidiPort* m_Port;
    MidiQueue* m_Queue;
};

#endif /*PLAYSMF_H_*/
