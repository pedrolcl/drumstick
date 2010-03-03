/*
    Cakewalk WRK file dump program
    Copyright (C) 2006-2010, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    Based on midifile.c by Tim Thompson, M.Czeiszperger and Greg Lee

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

#include "dumpwrk.h"
#include "qwrk.h"
#include <cstdlib>
#include <QObject>
#include <QString>
#include <QCoreApplication>
#include <QTextStream>
#include <QTextCodec>
#include <QStringList>

static QTextStream cout(stdout, QIODevice::WriteOnly);

const QString NO_CHANNEL("--");

QSpyWrk::QSpyWrk():
    m_currentTrack(0)
{
    m_engine = new QWrk(this);
    m_engine->setTextCodec(QTextCodec::codecForName("WIN1252"));

    connect(m_engine, SIGNAL(signalWRKError(const QString&)),
                      SLOT(errorHandler(const QString&)));
    connect(m_engine, SIGNAL(signalWRKUnknownChunk(int,const QByteArray&)),
                      SLOT(unknownChunk(int,const QByteArray&)));
    connect(m_engine, SIGNAL(signalWRKHeader(int,int)),
                      SLOT(fileHeader(int,int)));
    connect(m_engine, SIGNAL(signalWRKGlobalVars()),
                      SLOT(globalVars()));
    connect(m_engine, SIGNAL(signalWRKTrack(const QString&, const QString&, int,int,int,int,int,bool,bool,bool)),
                      SLOT(trackHeader(const QString&, const QString&, int,int,int,int,int,bool,bool,bool)));
    connect(m_engine, SIGNAL(signalWRKTimeBase(int)),
                      SLOT(timeBase(int)));
    connect(m_engine, SIGNAL(signalWRKNote(int,int,int,int,int,int)),
                      SLOT(noteEvent(int,int,int,int,int,int)));
    connect(m_engine, SIGNAL(signalWRKKeyPress(int,int,int,int,int)),
                      SLOT(keyPressEvent(int,int,int,int,int)));
    connect(m_engine, SIGNAL(signalWRKCtlChange(int,int,int,int,int)),
                      SLOT(ctlChangeEvent(int,int,int,int,int)));
    connect(m_engine, SIGNAL(signalWRKPitchBend(int,int,int,int)),
                      SLOT(pitchBendEvent(int,int,int,int)));
    connect(m_engine, SIGNAL(signalWRKProgram(int,int,int,int)),
                      SLOT(programEvent(int,int,int,int)));
    connect(m_engine, SIGNAL(signalWRKChanPress(int,int,int,int)),
                      SLOT(chanPressEvent(int,int,int,int)));
    connect(m_engine, SIGNAL(signalWRKSysexEvent(int,int,int)),
                      SLOT(sysexEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalWRKSysex(int,const QString&,bool,int,const QByteArray&)),
                      SLOT(sysexEventBank(int,const QString&,bool,int,const QByteArray&)));
    connect(m_engine, SIGNAL(signalWRKText(int,int,int,const QString&)),
                      SLOT(textEvent(int,int,int,const QString&)));
    connect(m_engine, SIGNAL(signalWRKTimeSig(int,int,int)),
                      SLOT(timeSigEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalWRKKeySig(int,int)),
                      SLOT(keySigEvent(int,int)));
    connect(m_engine, SIGNAL(signalWRKTempo(int,int)),
                      SLOT(tempoEvent(int,int)));
    connect(m_engine, SIGNAL(signalWRKThru(int,int,int,int,int,int)),
                      SLOT(thruMode(int,int,int,int,int,int)));
    connect(m_engine, SIGNAL(signalWRKTrackOffset(int,int)),
                      SLOT(trackOffset(int,int)));
    connect(m_engine, SIGNAL(signalWRKTrackReps(int,int)),
                      SLOT(trackReps(int,int)));
    connect(m_engine, SIGNAL(signalWRKTrackPatch(int,int)),
                      SLOT(trackPatch(int,int)));
    connect(m_engine, SIGNAL(signalWRKTimeFormat(int,int)),
                      SLOT(timeFormat(int,int)));
    connect(m_engine, SIGNAL(signalWRKComments(const QString&)),
                      SLOT(comments(const QString&)));
    connect(m_engine, SIGNAL(signalWRKVariableRecord(const QString&,const QByteArray&)),
                      SLOT(variableRecord(const QString&,const QByteArray&)));
    connect(m_engine, SIGNAL(signalWRKNewTrack(const QString&,int,int,int,int,int,bool,bool,bool)),
                      SLOT(newTrackHeader(const QString&,int,int,int,int,int,bool,bool,bool)));
    connect(m_engine, SIGNAL(signalWRKSoftVer(const QString&)),
                      SLOT(softVersion(const QString&)));
    connect(m_engine, SIGNAL(signalWRKTrackName(int,const QString&)),
                      SLOT(trackName(int,const QString&)));
    connect(m_engine, SIGNAL(signalWRKStringTable(const QStringList&)),
                      SLOT(stringTable(const QStringList&)));
    connect(m_engine, SIGNAL(signalWRKTrackVol(int,int)),
                      SLOT(trackVol(int,int)));
    connect(m_engine, SIGNAL(signalWRKTrackBank(int,int)),
                      SLOT(trackBank(int,int)));
    connect(m_engine, SIGNAL(signalWRKSegment(int,int,const QString&)),
                      SLOT(segment(int,int,const QString&)));
    connect(m_engine, SIGNAL(signalWRKChord(int,int,const QString&,const QByteArray&)),
                      SLOT(chord(int,int,const QString&,const QByteArray&)));
    connect(m_engine, SIGNAL(signalWRKExpression(int,int,int,const QString&)),
                      SLOT(expression(int,int,int,const QString&)));
    connect(m_engine, SIGNAL(signalWRKHairpin(int,int,int,int)),
                      SLOT(hairpin(int,int,int,int)));

    cout.setRealNumberNotation(QTextStream::FixedNotation);
    cout.setRealNumberPrecision(4);
}

void QSpyWrk::dump(const int time, const int track, const QString& chan, const QString& event, const QString& data)
{
    cout << right << qSetFieldWidth(7) << time;
    cout << qSetFieldWidth(0) << left << ' ';
    cout << qSetFieldWidth(5) << right << track;
    cout << qSetFieldWidth(3) << chan;
    cout << qSetFieldWidth(0) << left << ' ';
    cout << qSetFieldWidth(25) << event;
    cout << qSetFieldWidth(0) << ' ' << data << endl;
}

void QSpyWrk::dumpStr(const int time, const QString& event, const QString& data)
{
    cout << right << qSetFieldWidth(7) << time;
    cout << qSetFieldWidth(6) << NO_CHANNEL;
    cout << qSetFieldWidth(3) << NO_CHANNEL;
    cout << qSetFieldWidth(0) << left << ' ';
    cout << qSetFieldWidth(25) << event;
    cout << qSetFieldWidth(0) << ' ' << data << endl;
}

void QSpyWrk::dumpHex(const QByteArray& data)
{
    int i = 0, j = 0;
    QString s;
    while ( i < data.count() ) {
        s.clear();
        for (j = 0; j < 16 && i < data.count(); ++j) {
            quint8 c = data[i++];
            s += QString(" %1").arg(c & 0xff, 2, 16, QChar('0'));
        }
        cout << qSetFieldWidth(42) << ' '
             << qSetFieldWidth(0)  << s << endl;
    }
}

void QSpyWrk::errorHandler(const QString& errorStr)
{
    cout << "*** Warning! " << errorStr << endl;
}

void QSpyWrk::fileHeader(int verh, int verl)
{
    dumpStr(0, "WRK File Version", QString("%1.%2").arg(verh).arg(verl));
}

void QSpyWrk::globalVars()
{
    dumpStr(0, "Global Vars", QString());
    /*
    cout << qSetFieldWidth(42) << ' ' << qSetFieldWidth(0) << " Now=" << m_engine->getNow() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " From=" << m_engine->getFrom() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " Thru=" << m_engine->getThru() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " KeySig=" << m_engine->getKeySig() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " Clock=" << m_engine->getClock()  << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " AutoSave=" << m_engine->getAutoSave() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " PlayDelay=" << m_engine->getPlayDelay() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " ZeroCtrls=" << m_engine->getZeroCtrls() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " SendSPP=" << m_engine->getSendSPP() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " SendCont=" << m_engine->getSendCont() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " PatchSearch=" << m_engine->getPatchSearch() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " AutoStop=" << m_engine->getAutoStop() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " StopTime=" <<  m_engine->getStopTime() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " AutoRewind=" << m_engine->getAutoRewind() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " RewindTime=" << m_engine->getRewindTime() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " MetroPlay=" << m_engine->getMetroPlay() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " MetroRecord=" << m_engine->getMetroRecord() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " MetroAccent=" << m_engine->getMetroAccent() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " CountIn=" << m_engine->getCountIn() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " ThruOn=" << m_engine->getThruOn() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " AutoRestart=" << m_engine->getAutoRestart() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " CurTempoOfs=" << m_engine->getCurTempoOfs() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " TempoOfs1=" << m_engine->getTempoOfs1() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " TempoOfs2=" << m_engine->getTempoOfs2() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " TempoOfs3=" << m_engine->getTempoOfs3() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " PunchEnabled=" << m_engine->getPunchEnabled() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " PunchInTime=" << m_engine->getPunchInTime() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " PunchOutTime=" << m_engine->getPunchOutTime() << endl;
    cout << qSetFieldWidth(42)  << ' ' << qSetFieldWidth(0) << " EndAllTime=" << m_engine->getEndAllTime() << endl;
    */
}

void QSpyWrk::trackHeader( const QString& name1, const QString& name2,
                           int trackno, int channel, int pitch,
                           int velocity, int port,
                           bool selected, bool muted, bool loop )
{
    Q_UNUSED(pitch)
    Q_UNUSED(velocity)
    Q_UNUSED(port)
    Q_UNUSED(selected)
    Q_UNUSED(muted)
    Q_UNUSED(loop)
    m_currentTrack = trackno;
    dump(0, trackno, QString::number(channel), "Track", QString("name1='%2' name2='%3'").arg(name1).arg(name2));
}

void QSpyWrk::timeBase(int timebase)
{
    dumpStr(0, "Ticks per Quarter Note", QString::number(timebase));
}

void QSpyWrk::noteEvent(int track, int time, int chan, int pitch, int vol, int dur)
{
    dump(time, track, QString::number(chan), "Note", QString("key=%1 vel=%2 dur=%3").arg(pitch).arg(vol).arg(dur));
}

void QSpyWrk::keyPressEvent(int track, int time, int chan, int pitch, int press)
{
    dump(time, track, QString::number(chan), "Key Pressure", QString("key=%1 press=%2").arg(pitch).arg(press));
}

void QSpyWrk::ctlChangeEvent(int track, int time, int chan, int ctl, int value)
{
    dump(time, track, QString::number(chan), "Control Change", QString("ctl=%1 val=%2").arg(ctl).arg(value));
}

void QSpyWrk::pitchBendEvent(int track, int time, int chan, int value)
{
    dump(time, track, QString::number(chan), "Pitch Bend", QString::number(value));
}

void QSpyWrk::programEvent(int track, int time, int chan, int patch)
{
    dump(time, track, QString::number(chan), "Program Change", QString::number(patch));
}

void QSpyWrk::chanPressEvent(int track, int time, int chan, int press)
{
    dump(time, track, QString::number(chan), "Channel Pressure", QString::number(press));
}

void QSpyWrk::sysexEvent(int track, int time, int bank)
{
    dump(time, track, NO_CHANNEL, "System Exclusive", QString::number(bank));
}

void QSpyWrk::sysexEventBank(int bank, const QString& name, bool autosend, int port, const QByteArray& data)
{
    dumpStr(0, "System Exclusive Bank", QString("bank=%1 name='%2' auto=%3 port=%4").arg(bank).arg(name).arg(autosend).arg(port));
    dumpHex(data);
}

void QSpyWrk::forcedChannel(int channel)
{
    dump(0, 0, NO_CHANNEL, "Forced channel", QString::number(channel));
}

void QSpyWrk::forcedPort(int port)
{
    dump(0, 0, NO_CHANNEL, "Forced port", QString::number(port));
}

void QSpyWrk::textEvent(int track, int time, int typ, const QString& data)
{
    dump(time, track, NO_CHANNEL, QString("Text (%1)").arg(typ), data);
}

void QSpyWrk::timeSigEvent(int bar, int num, int den)
{
    dumpStr(0, "Time Signature", QString("bar=%1, %2/%3").arg(bar).arg(num).arg(den));
}

void QSpyWrk::keySigEvent(int bar, int alt)
{
    dumpStr(0, "Key Signature", QString("bar=%1, alt=%2").arg(bar).arg(alt));
}

void QSpyWrk::tempoEvent(int time, int tempo)
{
    dumpStr(time, "Tempo", QString::number(tempo));
}

void QSpyWrk::thruMode(int mode, int port, int channel, int keyPlus, int velPlus, int localPort)
{
    dumpStr(0, "Thru Mode", QString("mode=%1 port=%2 chan=%3 key+=%4 vel+=%5 port=%6").arg(mode).arg(port).arg(channel).arg(keyPlus).arg(velPlus).arg(localPort));
}

void QSpyWrk::trackOffset(int track, int ofs)
{
    dump(0, track, NO_CHANNEL, "Track Offset", QString::number(ofs));
}

void QSpyWrk::trackReps(int track, int reps)
{
    dump(0, track, NO_CHANNEL, "Track Repetitions", QString::number(reps));
}

void QSpyWrk::trackPatch(int track, int patch)
{
    dump(0, track, NO_CHANNEL, "Track Patch", QString::number(patch));
}

void QSpyWrk::timeFormat(int frsec, int ofs)
{
    dumpStr(0, "SMPTE Time Format", QString("%1 frames/second, offset=%2").arg(frsec).arg(ofs));
}

void QSpyWrk::comments(const QString& cmt)
{
    dumpStr(0, "Comment", cmt.trimmed());
}

void QSpyWrk::variableRecord(const QString& name, const QByteArray& data)
{
    QString s = name;
    bool isReadable = ( name == "Title" || name == "Author" ||
                       name == "Copyright" || name == "Subtitle" ||
                       name == "Instructions" || name == "Keywords" );
    if (isReadable) {
        s += ": ";
        if (m_engine->getTextCodec() == 0)
            s += QString(data);
        else
            s += m_engine->getTextCodec()->toUnicode(data);
    }
    dumpStr(0, "Variable Record", s.trimmed());
    if (!isReadable)
        dumpHex(data);
}

void QSpyWrk::newTrackHeader( const QString& name,
                              int trackno, int channel, int pitch,
                              int velocity, int port,
                              bool selected, bool muted, bool loop )
{
    Q_UNUSED(pitch)
    Q_UNUSED(velocity)
    Q_UNUSED(port)
    Q_UNUSED(selected)
    Q_UNUSED(muted)
    Q_UNUSED(loop)
    m_currentTrack = trackno;
    dump(0, trackno, QString::number(channel), "Track", name);
}

void QSpyWrk::softVersion(const QString& version)
{
    dumpStr(0, "Software Version", version);
}

void QSpyWrk::trackName(int trackno, const QString& name)
{
    dump(0, trackno, NO_CHANNEL, "Track Name", name);
}

void QSpyWrk::stringTable(const QStringList& table)
{
    dumpStr(0, "String Table", table.join(", "));
}

void QSpyWrk::trackVol(int track, int vol)
{
    dump(0, track, NO_CHANNEL, "Track Volume", QString::number(vol));
}

void QSpyWrk::trackBank(int track, int bank)
{
    dump(0, track, NO_CHANNEL, "Track Bank", QString::number(bank));
}

void QSpyWrk::segment(int track, int time, const QString& name)
{
    dump(time, track, NO_CHANNEL, "Track Segment", name);
}

void QSpyWrk::chord(int track, int time, const QString& name, const QByteArray& data)
{
    dump(time, track, NO_CHANNEL, "Chord Diagram", name);
    dumpHex(data);
}

void QSpyWrk::expression(int track, int time, int code, const QString& text)
{
    dump(time, track, NO_CHANNEL, "Expression", QString("text=%1 code=%2").arg(text).arg(code));
}

void QSpyWrk::hairpin(int track, int time, int code, int dur)
{
    dump(time, track, NO_CHANNEL, "Hairpin", QString("code=%1, dur=%2").arg(code).arg(dur));
}

void QSpyWrk::unknownChunk(int type, const QByteArray& data)
{
    QString name = QString("Unknown Chunk %1 (0x%2)").arg(type).arg(type, 2, 16, QChar('0'));
    dumpStr(0, name, QString("size=%2").arg(data.length()));
    dumpHex(data);
}

void QSpyWrk::run(QString fileName)
{
    cout << "__ticks track ch event____________________ data____" << endl;
    m_engine->readFromFile(fileName);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QSpyWrk spy;
    if (app.argc() > 1)
        spy.run(app.arguments().at(1));
    return 0;
}
