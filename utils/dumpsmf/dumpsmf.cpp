/*
    Standard MIDI File dump program
    Copyright (C) 2006-2020, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include "dumpsmf.h"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFileInfo>
#include <QObject>
#include <QString>
#include <QTextCodec>
#include <QTextStream>
#include <cstdlib>
#include <drumstick/qsmf.h>

QTextStream cout(stdout, QIODevice::WriteOnly);
QTextStream cerr(stderr, QIODevice::WriteOnly);

using drumstick::File::QSmf;

QSpySMF::QSpySMF():
    m_currentTrack(0), m_engine(nullptr), m_rc(0)
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    m_engine = new QSmf(this);
    m_engine->setTextCodec(codec);
    connect(m_engine, &QSmf::signalSMFHeader, this, &QSpySMF::headerEvent);
    connect(m_engine, &QSmf::signalSMFTrackStart, this, &QSpySMF::trackStartEvent);
    connect(m_engine, &QSmf::signalSMFTrackEnd, this, &QSpySMF::trackEndEvent);
    connect(m_engine, &QSmf::signalSMFNoteOn, this, &QSpySMF::noteOnEvent);
    connect(m_engine, &QSmf::signalSMFNoteOff, this, &QSpySMF::noteOffEvent);
    connect(m_engine, &QSmf::signalSMFKeyPress, this, &QSpySMF::keyPressEvent);
    connect(m_engine, &QSmf::signalSMFCtlChange, this, &QSpySMF::ctlChangeEvent);
    connect(m_engine, &QSmf::signalSMFPitchBend, this, &QSpySMF::pitchBendEvent);
    connect(m_engine, &QSmf::signalSMFProgram, this, &QSpySMF::programEvent);
    connect(m_engine, &QSmf::signalSMFChanPress, this, &QSpySMF::chanPressEvent);
    connect(m_engine, &QSmf::signalSMFSysex, this, &QSpySMF::sysexEvent);
    connect(m_engine, &QSmf::signalSMFSeqSpecific, this, &QSpySMF::seqSpecificEvent);
    connect(m_engine, &QSmf::signalSMFMetaUnregistered, this, &QSpySMF::metaMiscEvent);
    connect(m_engine, &QSmf::signalSMFSequenceNum, this, &QSpySMF::seqNum);
    connect(m_engine, &QSmf::signalSMFforcedChannel, this, &QSpySMF::forcedChannel);
    connect(m_engine, &QSmf::signalSMFforcedPort, this, &QSpySMF::forcedPort);
    connect(m_engine, &QSmf::signalSMFText, this, &QSpySMF::textEvent);
    connect(m_engine, &QSmf::signalSMFendOfTrack, this, &QSpySMF::endOfTrackEvent);
    connect(m_engine, &QSmf::signalSMFTimeSig, this, &QSpySMF::timeSigEvent);
    connect(m_engine, &QSmf::signalSMFSmpte, this, &QSpySMF::smpteEvent);
    connect(m_engine, &QSmf::signalSMFKeySig, this, &QSpySMF::keySigEvent);
    connect(m_engine, &QSmf::signalSMFTempo, this, &QSpySMF::tempoEvent);
    connect(m_engine, &QSmf::signalSMFError, this, &QSpySMF::errorHandler);
    cout.setRealNumberNotation(QTextStream::FixedNotation);
    cout.setRealNumberPrecision(4);
    cout.setCodec(codec);
}

void QSpySMF::dump(const QString& chan, const QString& event,
                   const QString& data)
{
    cout << right << qSetFieldWidth(7) << m_engine->getCurrentTime();
    cout << right << qSetFieldWidth(10) << m_engine->getRealTime() / 1600.0;
    cout << qSetFieldWidth(3) << chan;
    cout << qSetFieldWidth(0) << left << " ";
    cout << qSetFieldWidth(15) << event;
    cout << qSetFieldWidth(0) << " " << data << endl;
}

void QSpySMF::dumpStr(const QString& event, const QString& data)
{
    cout << right << qSetFieldWidth(7) << m_engine->getCurrentTime();
    cout << right << qSetFieldWidth(10) << m_engine->getRealTime() / 1600.0;
    cout << qSetFieldWidth(3) << "--";
    cout << qSetFieldWidth(0) << left << " ";
    cout << qSetFieldWidth(15) << event;
    cout << qSetFieldWidth(0) << " " << data << endl;
}

void QSpySMF::headerEvent(int format, int ntrks, int division)
{
    dumpStr("SMF Header", QString("Format=%1, Tracks=%2, Division=%3")
            .arg(format).arg(ntrks).arg(division));
}

void QSpySMF::trackStartEvent()
{
    m_currentTrack++;
    dumpStr("Track", QString("Start: %1").arg(m_currentTrack));
}

void QSpySMF::trackEndEvent()
{
    dumpStr("Track", QString("End: %1").arg(m_currentTrack));
}

void QSpySMF::endOfTrackEvent()
{
    dumpStr("Meta Event", "End Of Track");
}

void QSpySMF::noteOnEvent(int chan, int pitch, int vol)
{
    dump(QString("%1").arg(chan, 2), "Note On", QString("%1, %2").arg(pitch).arg(vol));
}

void QSpySMF::noteOffEvent(int chan, int pitch, int vol)
{
    dump(QString("%1").arg(chan, 2), "Note Off", QString("%1, %2").arg(pitch).arg(vol));
}

void QSpySMF::keyPressEvent(int chan, int pitch, int press)
{
    dump(QString("%1").arg(chan, 2), "Key Pressure", QString("%1, %2").arg(pitch).arg(press));
}

void QSpySMF::ctlChangeEvent(int chan, int ctl, int value)
{
    dump(QString("%1").arg(chan, 2), "Control Change", QString("%1, %2").arg(ctl).arg(value));
}

void QSpySMF::pitchBendEvent(int chan, int value)
{
    dump(QString("%1").arg(chan, 2), "Pitch Bend", QString::number(value));
}

void QSpySMF::programEvent(int chan, int patch)
{
    dump(QString("%1").arg(chan, 2), "Program Change", QString::number(patch));
}

void QSpySMF::chanPressEvent(int chan, int press)
{
    dump(QString("%1").arg(chan, 2), "Chan.Pressure", QString::number(press));
}

void QSpySMF::sysexEvent(const QByteArray& data)
{
    int j;
    QString s;
    for (j = 0; j < data.count(); ++j)
        s.append(QString("%1 ").arg(data[j] & 0xff, 2, 16));
    dumpStr("SysEx", s);
}

/*void QSpySMF::variableEvent(const QByteArray& data)
{
    int j;
    QString s;
    for (j = 0; j < data.count(); ++j)
        s.append(QString("%1 ").arg(data[j] & 0xff, 2, 16));
    dumpStr("Variable event", s);
}*/

void QSpySMF::seqSpecificEvent(const QByteArray& data)
{
    int j;
    QString s;
    for (j = 0; j < data.count(); ++j)
        s.append(QString("%1 ").arg(data[j] & 0xff, 2, 16));
    dumpStr("Seq. specific", s);
}

void QSpySMF::metaMiscEvent(int typ, const QByteArray& data)
{
    int j;
    QString s = QString("type=%1 ").arg(typ);
    for (j = 0; j < data.count(); ++j)
        s.append(QString("%1 ").arg(data[j] & 0xff, 2, 16));
    dumpStr("Meta (unreg.)", s);
}

void QSpySMF::seqNum(int seq)
{
    dump("--", "Sequence num.", QString::number(seq));
}

void QSpySMF::forcedChannel(int channel)
{
    dump("--", "Forced channel", QString::number(channel));
}

void QSpySMF::forcedPort(int port)
{
    dump("--", "Forced port", QString::number(port));
}

void QSpySMF::textEvent(int typ, const QString& data)
{
    dumpStr(QString("Text (%1)").arg(typ), data);
}

void QSpySMF::smpteEvent(int b0, int b1, int b2, int b3, int b4)
{
    dump("--", "SMPTE", QString("%1, %2, %3, %4, %5").arg(b0).arg(b1).arg(b2).arg(b3).arg(b4));
}

void QSpySMF::timeSigEvent(int b0, int b1, int b2, int b3)
{
    dump("--", "Time Signature", QString("%1, %2, %3, %4").arg(b0).arg(b1).arg(b2).arg(b3));
}

void QSpySMF::keySigEvent(int b0, int b1)
{
    dump("--", "Key Signature", QString("%1, %2").arg(b0).arg(b1));
}

void QSpySMF::tempoEvent(int tempo)
{
    dump("--", "Tempo", QString::number(tempo));
}

void QSpySMF::errorHandler(const QString& errorStr)
{
    m_rc++;
    cerr << "*** Warning! " << errorStr
         << " at file offset " << m_engine->getFilePos()
         << endl;
}

void QSpySMF::run(QString fileName)
{
    m_currentTrack = 0;
    cout << "__ticks __seconds ch event__________ data____" << endl;
    m_engine->readFromFile(fileName);
}

int QSpySMF::numErrors()
{
    return m_rc;
}

int main(int argc, char **argv)
{
    const QString PGM_NAME = QStringLiteral("drumstick-dumpsmf");
    const QString PGM_DESCRIPTION = QStringLiteral("Drumstick command line utility for decoding SMF (Standard MIDI) files");

    QSpySMF spy;
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(PGM_NAME);
    QCoreApplication::setApplicationVersion(QStringLiteral(QT_STRINGIFY(VERSION)));

    QCommandLineParser parser;
    parser.setApplicationDescription(PGM_DESCRIPTION);
    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();
    parser.addPositionalArgument("file", "Input SMF file name.", "files...");
    parser.process(app);

    if (parser.isSet(versionOption) || parser.isSet(helpOption)) {
        return 0;
    }

    QStringList fileNames, positionalArgs = parser.positionalArguments();
    if (positionalArgs.isEmpty()) {
        cerr << "Input file name(s) missing" << endl;
        parser.showHelp();
    }

    foreach(const QString& a, positionalArgs) {
        QFileInfo f(a);
        if (f.exists())
            fileNames += f.canonicalFilePath();
        else
            cerr << "File not found: " << a << endl;
    }

    foreach(const QString& file, fileNames) {
        spy.run(file);
    }
    return spy.numErrors();
}

