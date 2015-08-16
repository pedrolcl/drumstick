/*
    Standard MIDI File dump program
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

#include "dumpsmf.h"
#include "qsmf.h"
#include "cmdlineargs.h"

#include <cstdlib>
#include <QObject>
#include <QString>
#include <QCoreApplication>
#include <QTextStream>
#include <QTextCodec>

static QTextStream cout(stdout, QIODevice::WriteOnly);

QSpySMF::QSpySMF():
    m_currentTrack(0)
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    m_engine = new QSmf(this);
    m_engine->setTextCodec(codec);
    connect(m_engine, SIGNAL(signalSMFHeader(int,int,int)), this, SLOT(headerEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalSMFTrackStart()), this, SLOT(trackStartEvent()));
    connect(m_engine, SIGNAL(signalSMFTrackEnd()), this, SLOT(trackEndEvent()));
    connect(m_engine, SIGNAL(signalSMFNoteOn(int,int,int)), this, SLOT(noteOnEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalSMFNoteOff(int,int,int)), this, SLOT(noteOffEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalSMFKeyPress(int,int,int)), this, SLOT(keyPressEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalSMFCtlChange(int,int,int)), this, SLOT(ctlChangeEvent(int,int,int)));
    connect(m_engine, SIGNAL(signalSMFPitchBend(int,int)), this, SLOT(pitchBendEvent(int,int)));
    connect(m_engine, SIGNAL(signalSMFProgram(int,int)), this, SLOT(programEvent(int,int)));
    connect(m_engine, SIGNAL(signalSMFChanPress(int,int)), this, SLOT(chanPressEvent(int,int)));
    connect(m_engine, SIGNAL(signalSMFSysex(const QByteArray&)), this, SLOT(sysexEvent(const QByteArray&)));
    connect(m_engine, SIGNAL(signalSMFSeqSpecific(const QByteArray&)), this, SLOT(seqSpecificEvent(const QByteArray&)));
    connect(m_engine, SIGNAL(signalSMFMetaUnregistered(int, const QByteArray&)), this, SLOT(metaMiscEvent(int, const QByteArray&)));
    //connect(m_engine, SIGNAL(signalSMFVariable(const QByteArray&)), this, SLOT(variableEvent(const QByteArray&)));
    connect(m_engine, SIGNAL(signalSMFSequenceNum(int)), this, SLOT(seqNum(int)));
    connect(m_engine, SIGNAL(signalSMFforcedChannel(int)), this, SLOT(forcedChannel(int)));
    connect(m_engine, SIGNAL(signalSMFforcedPort(int)), this, SLOT(forcedPort(int)));
    connect(m_engine, SIGNAL(signalSMFText(int,const QString&)), this, SLOT(textEvent(int,const QString&)));
    connect(m_engine, SIGNAL(signalSMFendOfTrack()), this, SLOT(endOfTrackEvent()));
    connect(m_engine, SIGNAL(signalSMFTimeSig(int,int,int,int)), this, SLOT(timeSigEvent(int,int,int,int)));
    connect(m_engine, SIGNAL(signalSMFSmpte(int,int,int,int,int)), this, SLOT(smpteEvent(int,int,int,int,int)));
    connect(m_engine, SIGNAL(signalSMFKeySig(int,int)), this, SLOT(keySigEvent(int,int)));
    connect(m_engine, SIGNAL(signalSMFTempo(int)), this, SLOT(tempoEvent(int)));
    connect(m_engine, SIGNAL(signalSMFError(const QString&)), this, SLOT(errorHandler(const QString&)));
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
    cout << "*** Warning! " << errorStr
         << " at file offset " << m_engine->getFilePos()
         << endl;
}

void QSpySMF::run(QString fileName)
{
    cout << "__ticks __seconds ch event__________ data____" << endl;
    m_engine->readFromFile(fileName);
}

int main(int argc, char **argv)
{
    QSpySMF spy;
    CmdLineArgs args;
    args.setUsage("file");
    args.addRequiredArgument("file", "Input SMF name");
    args.parse(argc, argv);
    QVariant file = args.getArgument("file");
    if (!file.isNull())
        spy.run(file.toString());
}

