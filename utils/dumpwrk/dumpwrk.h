/*
    Cakewalk WRK file dump program
    Copyright (C) 2006-2024, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    Based on midifile.c by Tim Thompson, M.Czeiszperger and Greg Lee

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

#include <drumstick/qwrk.h>

  class QSpyWrk : public QObject
  {
    Q_OBJECT

  public:
    QSpyWrk();
    void run(QString fileName);
    void dump(const long time, const int track, const QString& chan, const QString& event, const QString& data);
    void dumpStr(const long time, const QString& event, const QString& data);
    void dumpHex(const QByteArray& data);
    void dumpVar(const QString& name, bool value);
    void dumpVar(const QString& name, int value);
    void dumpVar(const QString& name, unsigned int value);
    void setVerbosity(bool enable);
    bool verbosityEnabled() const;
    int returnCode() const { return m_rc; }

    static const QString NO_CHANNEL;

  public Q_SLOTS:
    void unknownChunk(int type, const QByteArray& data);
    void fileHeader(int verh, int verl);
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
    void forcedChannel(int channel);
    void forcedPort(int port);
    void textEvent(int track, long time, int typ, const QString& data);
    void timeSigEvent(int bar, int num, int den);
    void keySigEvent(int bar, int alt);
    void tempoEvent(long time, int tempo);
    void errorHandler(const QString& errorStr);
    void thruMode(int mode, int port, int channel, int keyPlus, int velPlus, int localPort);
    void trackOffset(int track, int ofs);
    void trackReps(int track, int reps);
    void trackPatch(int track, int patch);
    void timeFormat(int frsec, int ofs);
    void comments(const QString& cmt);
    void variableRecord(const QString& name, const QByteArray& data);
    void newTrackHeader(const QString& name,
                        int trackno, int channel, int pitch,
                        int velocity, int port,
                        bool selected, bool muted, bool loop);
    void softVersion(const QString& version);
    void trackName(int trackno, const QString& name);
    void stringTable(const QStringList& table);
    void trackVol(int track, int vol);
    void trackBank(int track, int bank);
    void segment(int track, long time, const QString& name);
    void chord(int track, long time, const QString& name, const QByteArray& data);
    void expression(int track, long time, int code, const QString& text);
    void hairpin(int track, long time, int code, int dur);
    void marker(long time, int smpte, const QString& text);

  private:
    bool m_verbosity;
    drumstick::File::QWrk *m_engine;
    int m_rc;
  };
