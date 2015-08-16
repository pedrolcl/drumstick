/*
    Overture OVE file dump program
    Copyright (C) 2006-2015, Fan Rui <vanferry@gmail.com>

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

#include "qove.h"

using namespace drumstick;

  class QSpyOve : public QObject
  {
    Q_OBJECT

  public:
    QSpyOve();
    void run(QString fileName);
    void dump(const long time, const int track, const QString& chan, const QString& event, const QString& data);
    void dumpStr(const long time, const QString& event, const QString& data);
    void dumpStr(const QString& event, const QString& data);
    void dumpHex(const QByteArray& data);
    void dumpVar(const QString& name, bool value);
    void dumpVar(const QString& name, int value);
    void dumpVar(const QString& name, unsigned int value);
    void setVerbosity(bool enable);
    bool verbosityEnabled() const;

  public slots:
    void errorHandler(const QString& errorStr);
    void fileHeader(int verh, int verl);
    void fileEnd();
/*
    void trackHeader(const QString& name1, const QString& name2,
                     int track, int channel, int pitch,
                     int velocity, int port,
                     bool selected, bool muted, bool loop);
*/
    void newTrackHeader(const QString& name,
                        int track, int channel, int pitch,
                        int velocity, int port,
                        bool selected, bool muted, bool loop);
    void timeBase(int timebase);
    void noteOnEvent(int track, long tick, int channel, int pitch, int vol);
    void noteOffEvent(int track, long tick, int channel, int pitch, int vol);
    void keyPressEvent(int track, long tick, int channel, int pitch, int press);
    void ctlChangeEvent(int track, long tick, int channel, int ctl, int value);
    void pitchBendEvent(int track, long tick, int channel, int value);
    void programEvent(int track, long tick, int channel, int patch);
    void chanPressEvent(int track, long tick, int channel, int press);
    void sysexEvent(int track, long tick, int bank);
    void sysexEventBank(int bank, const QString& name, bool autosend, int port, const QByteArray& data);
    void forcedChannel(int channel);
    void forcedPort(int port);
    void textEvent(int track, long tick, const QString& data);
    void timeSigEvent(int bar, long tick, int num, int den);
    void keySigEvent(int bar, long tick, int alt);
    void tempoEvent(long tick, int tempo);
    void trackPatch(int track, int channel, int patch);
    void trackVol(int track, int channel, int vol);
    void trackBank(int track, int channel, int bank);
    void chord(int track, long tick, const QString& name, const QByteArray& data);
    void expression(int track, long tick, int code, const QString& text);

  private:
    bool m_verbosity;
    QOve *m_engine;
  };
