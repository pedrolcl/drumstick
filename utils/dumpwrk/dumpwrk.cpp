/*
    Cakewalk WRK file dump program
    Copyright (C) 2006-2025, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include "dumpwrk.h"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFileInfo>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTextCodec>
#include <QTextStream>
#include <QVariant>
#include <cstdlib>
#include <drumstick/qwrk.h>

DISABLE_WARNING_PUSH
DISABLE_WARNING_DEPRECATED_DECLARATIONS

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#define right Qt::right
#define left Qt::left
#define endl Qt::endl
#endif

QTextStream cout(stdout, QIODevice::WriteOnly);
QTextStream cerr(stderr, QIODevice::WriteOnly);

using drumstick::File::QWrk;

const QString QSpyWrk::NO_CHANNEL = QStringLiteral("--");

QSpyWrk::QSpyWrk():
    m_verbosity(false),
    m_engine(nullptr),
    m_rc(0)
{
    m_engine = new QWrk(this);
    m_engine->setTextCodec(QTextCodec::codecForName("Windows-1252"));

    connect(m_engine, &QWrk::signalWRKError, this, &QSpyWrk::errorHandler);
    connect(m_engine, &QWrk::signalWRKUnknownChunk, this, &QSpyWrk::unknownChunk);
    connect(m_engine, &QWrk::signalWRKHeader, this, &QSpyWrk::fileHeader);
    connect(m_engine, &QWrk::signalWRKGlobalVars, this, &QSpyWrk::globalVars);
    connect(m_engine, &QWrk::signalWRKTrack, this, &QSpyWrk::trackHeader);
    connect(m_engine, &QWrk::signalWRKTimeBase, this, &QSpyWrk::timeBase);
    connect(m_engine, &QWrk::signalWRKNote, this, &QSpyWrk::noteEvent);
    connect(m_engine, &QWrk::signalWRKKeyPress, this, &QSpyWrk::keyPressEvent);
    connect(m_engine, &QWrk::signalWRKCtlChange, this, &QSpyWrk::ctlChangeEvent);
    connect(m_engine, &QWrk::signalWRKPitchBend, this, &QSpyWrk::pitchBendEvent);
    connect(m_engine, &QWrk::signalWRKProgram, this, &QSpyWrk::programEvent);
    connect(m_engine, &QWrk::signalWRKChanPress, this, &QSpyWrk::chanPressEvent);
    connect(m_engine, &QWrk::signalWRKSysexEvent, this, &QSpyWrk::sysexEvent);
    connect(m_engine, &QWrk::signalWRKSysex, this, &QSpyWrk::sysexEventBank);
    connect(m_engine, &QWrk::signalWRKText, this, &QSpyWrk::textEvent);
    connect(m_engine, &QWrk::signalWRKTimeSig, this, &QSpyWrk::timeSigEvent);
    connect(m_engine, &QWrk::signalWRKKeySig, this, &QSpyWrk::keySigEvent);
    connect(m_engine, &QWrk::signalWRKTempo, this, &QSpyWrk::tempoEvent);
    connect(m_engine, &QWrk::signalWRKThru, this, &QSpyWrk::thruMode);
    connect(m_engine, &QWrk::signalWRKTrackOffset, this, &QSpyWrk::trackOffset);
    connect(m_engine, &QWrk::signalWRKTrackReps, this, &QSpyWrk::trackReps);
    connect(m_engine, &QWrk::signalWRKTrackPatch, this, &QSpyWrk::trackPatch);
    connect(m_engine, &QWrk::signalWRKTimeFormat, this, &QSpyWrk::timeFormat);
    connect(m_engine, &QWrk::signalWRKComments, this, &QSpyWrk::comments);
    connect(m_engine, &QWrk::signalWRKVariableRecord, this, &QSpyWrk::variableRecord);
    connect(m_engine, &QWrk::signalWRKNewTrack, this, &QSpyWrk::newTrackHeader);
    connect(m_engine, &QWrk::signalWRKSoftVer, this, &QSpyWrk::softVersion);
    connect(m_engine, &QWrk::signalWRKTrackName, this, &QSpyWrk::trackName);
    connect(m_engine, &QWrk::signalWRKStringTable, this, &QSpyWrk::stringTable);
    connect(m_engine, &QWrk::signalWRKTrackVol, this, &QSpyWrk::trackVol);
    connect(m_engine, &QWrk::signalWRKTrackBank, this, &QSpyWrk::trackBank);
    connect(m_engine, &QWrk::signalWRKSegment, this, &QSpyWrk::segment);
    connect(m_engine, &QWrk::signalWRKChord, this, &QSpyWrk::chord);
    connect(m_engine, &QWrk::signalWRKExpression, this, &QSpyWrk::expression);
    connect(m_engine, &QWrk::signalWRKHairpin, this, &QSpyWrk::hairpin);
    connect(m_engine, &QWrk::signalWRKMarker, this, &QSpyWrk::marker);

    cout.setRealNumberNotation(QTextStream::FixedNotation);
    cout.setRealNumberPrecision(4);
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
    cout.setCodec("UTF-8");
#else
    cout.setEncoding(QStringConverter::Utf8);
#endif
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
                quint8 c = static_cast<quint8>(data[i++]);
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
    m_rc++;
    cerr << "*** Warning! " << errorStr << endl;
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
    dump(0, trackno, QString::number(channel), "Track", QString("name1='%2' name2='%3'").arg(name1, name2));
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
        if (m_engine->getTextCodec() == nullptr)
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

void QSpyWrk::marker(long time, int smpte, const QString &text)
{
    dumpStr(time, smpte == 0 ? "Marker" : "SMPTE Marker", text);
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
    const QString PGM_NAME = QStringLiteral("drumstick-dumpwrk");
    const QString PGM_DESCRIPTION = QStringLiteral("Drumstick command line utility for decoding WRK (Cakewalk) files");

    QSpyWrk spy;
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(PGM_NAME);
    QCoreApplication::setApplicationVersion(QStringLiteral(QT_STRINGIFY(VERSION)));

    QCommandLineParser parser;
    parser.setApplicationDescription(PGM_DESCRIPTION);
    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();
    QCommandLineOption verboseOption("verbose", "Verbose output.");
    parser.addOption(verboseOption);
    parser.addPositionalArgument("file", "Input WRK File Name(s).", "files...");
    parser.process(app);

    if (parser.isSet(versionOption) || parser.isSet(helpOption)) {
        return 0;
    }

    if (parser.isSet(verboseOption)) {
        spy.setVerbosity(true);
    }

    QStringList fileNames, positionalArgs = parser.positionalArguments();
    foreach(const QVariant& a, positionalArgs) {
        QFileInfo f(a.toString());
        if (f.exists())
            fileNames += f.canonicalFilePath();
        else
            cerr << "File not found: " << a.toString() << endl;
    }
    foreach(const QString& file, fileNames) {
        spy.run(file);
    }
    return spy.returnCode();
}

DISABLE_WARNING_POP
