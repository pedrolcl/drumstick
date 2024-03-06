/*
    Standard RIFF MIDI File dump program
    Copyright (C) 2006-2024, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <drumstick/qsmf.h>
#include <drumstick/rmid.h>
#include "riff.h"

class DumpRmid : public QObject
{
    Q_OBJECT

public:
    DumpRmid();
    void run(QString fileName);
    void dump(const QString& chan, const QString& event, const QString& data);
    void dumpStr(const QString& event, const QString& data);
    int  numErrors();
    void setExtract(bool enable);
    void extractFileData(const QString& fileSuffix, const QByteArray& data);

public Q_SLOTS:
    void headerEvent(int format, int ntrks, int division);
    void trackStartEvent();
    void trackEndEvent();
    void endOfTrackEvent();
    void noteOnEvent(int chan, int pitch, int vol);
    void noteOffEvent(int chan, int pitch, int vol);
    void keyPressEvent(int chan, int pitch, int press);
    void ctlChangeEvent(int chan, int ctl, int value);
    void pitchBendEvent(int chan, int value);
    void programEvent(int chan, int patch);
    void chanPressEvent(int chan, int press);
    void sysexEvent(const QByteArray& data);
    void seqSpecificEvent(const QByteArray& data);
    void metaMiscEvent(int typ, const QByteArray& data);
    void seqNum(int seq);
    void forcedChannel(int channel);
    void forcedPort(int port);
    void textEvent(int typ, const QString& data);
    void smpteEvent(int b0, int b1, int b2, int b3, int b4);
    void timeSigEvent(int b0, int b1, int b2, int b3);
    void keySigEvent(int b0, int b1);
    void tempoEvent(int tempo);
    void errorHandler(const QString& errorStr);

    void infoHandler(const QString& infoType, const QByteArray& data);
    void dataHandler(const QString& dataType, const QByteArray& data);

    void processDLS(QString name, QString version, QString copyright);
    void processInstrument(int bank, int pc, QString name);
    void processPercussion(int bank, int pc, QString name);

private:
    int m_currentTrack;
    drumstick::File::Rmidi *m_engine;
    drumstick::File::QSmf *m_smf;
    Riff *m_riff;
    int m_rc;
    bool m_extract;
    QString m_fileName;
};
