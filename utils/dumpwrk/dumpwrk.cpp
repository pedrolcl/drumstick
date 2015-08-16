/*
    Cakewalk WRK file dump program
    Copyright (C) 2006-2015, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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
#include "cmdlineargs.h"

#include <cstdlib>
#include <QObject>
#include <QString>
#include <QCoreApplication>
#include <QTextStream>
#include <QTextCodec>
#include <QStringList>
#include <QFileInfo>
#include <QVariant>

static QTextStream cout(stdout, QIODevice::WriteOnly);

const QString NO_CHANNEL("--");

QSpyWrk::QSpyWrk():
    m_verbosity(false),
    m_engine(0)
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
    connect(m_engine, SIGNAL(signalWRKNote(int,long,int,int,int,int)),
                      SLOT(noteEvent(int,long,int,int,int,int)));
    connect(m_engine, SIGNAL(signalWRKKeyPress(int,long,int,int,int)),
                      SLOT(keyPressEvent(int,long,int,int,int)));
    connect(m_engine, SIGNAL(signalWRKCtlChange(int,long,int,int,int)),
                      SLOT(ctlChangeEvent(int,long,int,int,int)));
    connect(m_engine, SIGNAL(signalWRKPitchBend(int,long,int,int)),
                      SLOT(pitchBendEvent(int,long,int,int)));
    connect(m_engine, SIGNAL(signalWRKProgram(int,long,int,int)),
                      SLOT(programEvent(int,long,int,int)));
    connect(m_engine, SIGNAL(signalWRKChanPress(int,long,int,int)),
                      SLOT(chanPressEvent(int,long,int,int)));
    connect(m_engine, SIGNAL(signalWRKSysexEvent(int,long,int)),
                      SLOT(sysexEvent(int,long,int)));
    connect(m_engine, SIGNAL(signalWRKSysex(int,const QString&,bool,int,const QByteArray&)),
                      SLOT(sysexEventBank(int,const QString&,bool,int,const QByteArray&)));
    connect(m_engine, SIGNAL(signalWRKText(int,long,int,const QString&)),
                      SLOT(textEvent(int,long,int,const QString&)));
    connect(m_engine, SIGNAL(signalWRKTimeSig(int,int,int)),
                      SLOT(timeSigEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalWRKKeySig(int,int)),
                      SLOT(keySigEvent(int,int)));
    connect(m_engine, SIGNAL(signalWRKTempo(long,int)),
                      SLOT(tempoEvent(long,int)));
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
    connect(m_engine, SIGNAL(signalWRKSegment(int,long,const QString&)),
                      SLOT(segment(int,long,const QString&)));
    connect(m_engine, SIGNAL(signalWRKChord(int,long,const QString&,const QByteArray&)),
                      SLOT(chord(int,long,const QString&,const QByteArray&)));
    connect(m_engine, SIGNAL(signalWRKExpression(int,long,int,const QString&)),
                      SLOT(expression(int,long,int,const QString&)));
    connect(m_engine, SIGNAL(signalWRKHairpin(int,long,int,int)),
                      SLOT(hairpin(int,long,int,int)));

    cout.setRealNumberNotation(QTextStream::FixedNotation);
    cout.setRealNumberPrecision(4);
    cout.setCodec("UTF-8");
}

void QSpyWrk::dump(const long time, const int track, const QString& chan, const QString& event, const QString& data)
{
    cout << right << qSetFieldWidth(7) << time;
    cout << qSetFieldWidth(0) << left << ' ';
    cout << qSetFieldWidth(5) << right << track;
    cout << qSetFieldWidth(3) << chan;
    cout << qSetFieldWidth(0) << left << ' ';
    cout << qSetFieldWidth(25) << event;
    cout << qSetFieldWidth(0) << ' ' << data << endl;
}

void QSpyWrk::dumpStr(const long time, const QString& event, const QString& data)
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
    if (m_verbosity) {
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
}

void QSpyWrk::dumpVar(const QString& name, int value)
{
    cout << qSetFieldWidth(43) << ' '
         << qSetFieldWidth(0) << name << " = "
         << value << endl;
}

void QSpyWrk::dumpVar(const QString& name, unsigned int value)
{
    cout << qSetFieldWidth(43) << ' '
         << qSetFieldWidth(0) << name << " = "
         << value << endl;
}

void QSpyWrk::dumpVar(const QString& name, bool value)
{
    cout << qSetFieldWidth(43) << ' '
         << qSetFieldWidth(0) << name << " = "
         << (value ? "true" : "false" ) << endl;
}

void QSpyWrk::setVerbosity(bool enabled)
{
    m_verbosity = enabled;
}

bool QSpyWrk::verbosityEnabled() const
{
    return m_verbosity;
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
    if (m_verbosity) {
        dumpVar("Now", m_engine->getNow());
        dumpVar("From", m_engine->getFrom());
        dumpVar("Thru", m_engine->getThru());
        dumpVar("KeySig", m_engine->getKeySig());
        dumpVar("Clock", m_engine->getClock());
        dumpVar("AutoSave", m_engine->getAutoSave());
        dumpVar("PlayDelay", m_engine->getPlayDelay());
        dumpVar("ZeroCtrls", m_engine->getZeroCtrls());
        dumpVar("SendSPP", m_engine->getSendSPP());
        dumpVar("SendCont", m_engine->getSendCont());
        dumpVar("PatchSearch", m_engine->getPatchSearch());
        dumpVar("AutoStop", m_engine->getAutoStop());
        dumpVar("StopTime",  m_engine->getStopTime());
        dumpVar("AutoRewind", m_engine->getAutoRewind());
        dumpVar("RewindTime", m_engine->getRewindTime());
        dumpVar("MetroPlay", m_engine->getMetroPlay());
        dumpVar("MetroRecord", m_engine->getMetroRecord());
        dumpVar("MetroAccent", m_engine->getMetroAccent());
        dumpVar("CountIn", m_engine->getCountIn());
        dumpVar("ThruOn", m_engine->getThruOn());
        dumpVar("AutoRestart", m_engine->getAutoRestart());
        dumpVar("CurTempoOfs", m_engine->getCurTempoOfs());
        dumpVar("TempoOfs1", m_engine->getTempoOfs1());
        dumpVar("TempoOfs2", m_engine->getTempoOfs2());
        dumpVar("TempoOfs3", m_engine->getTempoOfs3());
        dumpVar("PunchEnabled", m_engine->getPunchEnabled());
        dumpVar("PunchInTime", m_engine->getPunchInTime());
        dumpVar("PunchOutTime", m_engine->getPunchOutTime());
        dumpVar("EndAllTime", m_engine->getEndAllTime());
    }
}

void QSpyWrk::trackHeader( const QString& name1, const QString& name2,
                           int trackno, int channel, int pitch,
                           int velocity, int port,
                           bool selected, bool muted, bool loop )
{
    dump(0, trackno, QString::number(channel), "Track", QString("name1='%2' name2='%3'").arg(name1).arg(name2));
    if (m_verbosity) {
        dumpVar("pitch", pitch);
        dumpVar("velocity",velocity);
        dumpVar("port", port);
        dumpVar("selected", selected);
        dumpVar("muted", muted);
        dumpVar("loop", loop);
    }
}

void QSpyWrk::timeBase(int timebase)
{
    dumpStr(0, "Ticks per Quarter Note", QString::number(timebase));
}

void QSpyWrk::noteEvent(int track, long time, int chan, int pitch, int vol, int dur)
{
    dump(time, track, QString::number(chan), "Note", QString("key=%1 vel=%2 dur=%3").arg(pitch).arg(vol).arg(dur));
}

void QSpyWrk::keyPressEvent(int track, long time, int chan, int pitch, int press)
{
    dump(time, track, QString::number(chan), "Key Pressure", QString("key=%1 press=%2").arg(pitch).arg(press));
}

void QSpyWrk::ctlChangeEvent(int track, long time, int chan, int ctl, int value)
{
    dump(time, track, QString::number(chan), "Control Change", QString("ctl=%1 val=%2").arg(ctl).arg(value));
}

void QSpyWrk::pitchBendEvent(int track, long time, int chan, int value)
{
    dump(time, track, QString::number(chan), "Pitch Bend", QString::number(value));
}

void QSpyWrk::programEvent(int track, long time, int chan, int patch)
{
    dump(time, track, QString::number(chan), "Program Change", QString::number(patch));
}

void QSpyWrk::chanPressEvent(int track, long time, int chan, int press)
{
    dump(time, track, QString::number(chan), "Channel Pressure", QString::number(press));
}

void QSpyWrk::sysexEvent(int track, long time, int bank)
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

void QSpyWrk::textEvent(int track, long time, int typ, const QString& data)
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

void QSpyWrk::tempoEvent(long time, int tempo)
{
    double bpm = tempo / 100.0;
    dumpStr(time, "Tempo", QString::number(bpm, 'f', 2));
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
    dump(0, trackno, QString::number(channel), "Track", name);
    if (m_verbosity) {
        dumpVar("pitch", pitch);
        dumpVar("velocity",velocity);
        dumpVar("port", port);
        dumpVar("selected", selected);
        dumpVar("muted", muted);
        dumpVar("loop", loop);
    }
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

void QSpyWrk::segment(int track, long time, const QString& name)
{
    dump(time, track, NO_CHANNEL, "Track Segment", name);
}

void QSpyWrk::chord(int track, long time, const QString& name, const QByteArray& data)
{
    dump(time, track, NO_CHANNEL, "Chord Diagram", name);
    dumpHex(data);
}

void QSpyWrk::expression(int track, long time, int code, const QString& text)
{
    dump(time, track, NO_CHANNEL, "Expression", QString("text=%1 code=%2").arg(text).arg(code));
}

void QSpyWrk::hairpin(int track, long time, int code, int dur)
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
    QSpyWrk spy;
    CmdLineArgs args;
    args.setUsage("[options] file");
    args.addOption('v', "verbose", "Verbose output");
    args.addRequiredArgument("file", "Input WRK file name");
    args.parse(argc, argv);

    QVariant verbose = args.getOption("verbose");
    if (!verbose.isNull())
        spy.setVerbosity(true);

    QVariantList files = args.getArguments("file");
    QStringList fileNames;
    foreach(const QVariant& a, files) {
        QFileInfo f(a.toString());
        if (f.exists())
            fileNames += f.canonicalFilePath();
        else
            cout << "File not found: " << a.toString() << endl;
    }

    foreach(const QString& file, fileNames) {
        spy.run(file);
    }

    return 0;
}
