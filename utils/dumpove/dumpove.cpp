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

#include "dumpove.h"
#include "qove.h"
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

QSpyOve::QSpyOve():
    m_verbosity(false),
    m_engine(0)
{
    m_engine = new QOve(this);

    connect(m_engine, SIGNAL(signalOVEError(const QString&)),
                      SLOT(errorHandler(const QString&)));
    connect(m_engine, SIGNAL(signalOVEHeader(int,int)),
                      SLOT(fileHeader(int,int)));
    connect(m_engine, SIGNAL(signalOVEEnd()),
                      SLOT(fileEnd()));
    connect(m_engine, SIGNAL(signalOVENoteOn(int, long, int, int, int)),
					  SLOT(noteOnEvent(int, long, int, int, int)));
    connect(m_engine, SIGNAL(signalOVENoteOff(int, long, int, int, int)),
    				  SLOT(noteOffEvent(int, long, int, int, int)));
    connect(m_engine, SIGNAL(signalOVEKeyPress(int,long,int,int,int)),
                      SLOT(keyPressEvent(int,long,int,int,int)));
    connect(m_engine, SIGNAL(signalOVECtlChange(int,long,int,int,int)),
                      SLOT(ctlChangeEvent(int,long,int,int,int)));
    connect(m_engine, SIGNAL(signalOVEPitchBend(int,long,int,int)),
                      SLOT(pitchBendEvent(int,long,int,int)));
    connect(m_engine, SIGNAL(signalOVEProgram(int,long,int,int)),
                      SLOT(programEvent(int,long,int,int)));
    connect(m_engine, SIGNAL(signalOVEChanPress(int,long,int,int)),
                      SLOT(chanPressEvent(int,long,int,int)));
    connect(m_engine, SIGNAL(signalOVESysexEvent(int,long,int)),
                      SLOT(sysexEvent(int,long,int)));
    connect(m_engine, SIGNAL(signalOVESysex(int,const QString&,bool,int,const QByteArray&)),
                      SLOT(sysexEventBank(int,const QString&,bool,int,const QByteArray&)));
    connect(m_engine, SIGNAL(signalOVEText(int,long,const QString&)),
                      SLOT(textEvent(int,long,const QString&)));
    connect(m_engine, SIGNAL(signalOVETimeSig(int,long,int,int)),
                      SLOT(timeSigEvent(int,long,int,int)));
    connect(m_engine, SIGNAL(signalOVEKeySig(int,long,int)),
                      SLOT(keySigEvent(int,long,int)));
    connect(m_engine, SIGNAL(signalOVETempo(long,int)),
                      SLOT(tempoEvent(long,int)));
    connect(m_engine, SIGNAL(signalOVETrackPatch(int,int,int)),
                      SLOT(trackPatch(int,int,int)));
    connect(m_engine, SIGNAL(signalOVENewTrack(const QString&,int,int,int,int,int,bool,bool,bool)),
                      SLOT(newTrackHeader(const QString&,int,int,int,int,int,bool,bool,bool)));
    connect(m_engine, SIGNAL(signalOVETrackVol(int,int,int)),
                      SLOT(trackVol(int,int,int)));
    connect(m_engine, SIGNAL(signalOVETrackBank(int,int,int)),
                      SLOT(trackBank(int,int,int)));
    connect(m_engine, SIGNAL(signalOVEChord(int,long,const QString&,const QByteArray&)),
                      SLOT(chord(int,long,const QString&,const QByteArray&)));
    connect(m_engine, SIGNAL(signalOVEExpression(int,long,int,const QString&)),
                      SLOT(expression(int,long,int,const QString&)));

    cout.setRealNumberNotation(QTextStream::FixedNotation);
    cout.setRealNumberPrecision(4);
}

void QSpyOve::dump(const long tick, const int track, const QString& chan, const QString& event, const QString& data)
{
    cout << right << qSetFieldWidth(7) << tick;
    cout << qSetFieldWidth(0) << left << ' ';
    cout << qSetFieldWidth(5) << right << track;
    cout << qSetFieldWidth(3) << chan;
    cout << qSetFieldWidth(0) << left << ' ';
    cout << qSetFieldWidth(25) << event;
    cout << qSetFieldWidth(0) << ' ' << data << endl;
}

void QSpyOve::dumpStr(const long tick, const QString& event, const QString& data)
{
    cout << right << qSetFieldWidth(7) << tick;
    cout << qSetFieldWidth(6) << NO_CHANNEL;
    cout << qSetFieldWidth(3) << NO_CHANNEL;
    cout << qSetFieldWidth(0) << left << ' ';
    cout << qSetFieldWidth(25) << event;
    cout << qSetFieldWidth(0) << ' ' << data << endl;
}

void QSpyOve::dumpStr(const QString& event, const QString& data)
{
    cout << qSetFieldWidth(6) << NO_CHANNEL;
    cout << qSetFieldWidth(3) << NO_CHANNEL;
    cout << qSetFieldWidth(0) << left << ' ';
    cout << qSetFieldWidth(25) << event;
    cout << qSetFieldWidth(0) << ' ' << data << endl;
}

void QSpyOve::dumpHex(const QByteArray& data)
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

void QSpyOve::dumpVar(const QString& name, int value)
{
    cout << qSetFieldWidth(43) << ' '
         << qSetFieldWidth(0) << name << " = "
         << value << endl;
}

void QSpyOve::dumpVar(const QString& name, unsigned int value)
{
    cout << qSetFieldWidth(43) << ' '
         << qSetFieldWidth(0) << name << " = "
         << value << endl;
}

void QSpyOve::dumpVar(const QString& name, bool value)
{
    cout << qSetFieldWidth(43) << ' '
         << qSetFieldWidth(0) << name << " = "
         << (value ? "true" : "false" ) << endl;
}

void QSpyOve::setVerbosity(bool enabled)
{
    m_verbosity = enabled;
}

bool QSpyOve::verbosityEnabled() const
{
    return m_verbosity;
}

void QSpyOve::errorHandler(const QString& errorStr)
{
    cout << "*** Warning! " << errorStr << endl;
}

void QSpyOve::fileHeader(int res, int tracks)
{
    dumpStr(0, "OVE File Header",
            QString("Resolution=%1 Tracks=%2").arg(res).arg(tracks));
}

void QSpyOve::fileEnd()
{
	dumpStr(0, "Ove File End", QString());
}

void QSpyOve::newTrackHeader( const QString& name,
                              int track, int channel, int pitch,
                              int velocity, int port,
                              bool selected, bool muted, bool loop )
{
    dump(0, track, QString::number(channel), "Track", name);
    if (m_verbosity) {
        dumpVar("pitch", pitch);
        dumpVar("velocity",velocity);
        dumpVar("port", port);
        dumpVar("selected", selected);
        dumpVar("muted", muted);
        dumpVar("loop", loop);
    }
}

void QSpyOve::timeBase(int timebase)
{
    dumpStr(0, "Ticks per Quarter Note", QString::number(timebase));
}

void QSpyOve::noteOnEvent(int track, long tick, int channel, int pitch, int vol) {
	dump(tick, track, QString::number(channel), "Note On", QString("key=%1 vel=%2").arg(pitch).arg(vol));
}

void QSpyOve::noteOffEvent(int track, long tick, int channel, int pitch, int vol) {
	dump(tick, track, QString::number(channel), "Note Off", QString("key=%1 vel=%2").arg(pitch).arg(vol));
}

void QSpyOve::keyPressEvent(int track, long tick, int channel, int pitch, int press)
{
    dump(tick, track, QString::number(channel), "Key Pressure", QString("key=%1 press=%2").arg(pitch).arg(press));
}

void QSpyOve::ctlChangeEvent(int track, long tick, int channel, int ctl, int value)
{
    dump(tick, track, QString::number(channel), "Control Change", QString("ctl=%1 val=%2").arg(ctl).arg(value));
}

void QSpyOve::pitchBendEvent(int track, long tick, int channel, int value)
{
    dump(tick, track, QString::number(channel), "Pitch Bend", QString::number(value));
}

void QSpyOve::programEvent(int track, long tick, int channel, int patch)
{
    dump(tick, track, QString::number(channel), "Program Change", QString::number(patch));
}

void QSpyOve::chanPressEvent(int track, long tick, int channel, int press)
{
    dump(tick, track, QString::number(channel), "Channel Pressure", QString::number(press));
}

void QSpyOve::sysexEvent(int track, long tick, int bank)
{
    dump(tick, track, NO_CHANNEL, "System Exclusive", QString::number(bank));
}

void QSpyOve::sysexEventBank(int bank, const QString& name, bool autosend, int port, const QByteArray& data)
{
    dumpStr(0, "System Exclusive Bank", QString("bank=%1 name='%2' auto=%3 port=%4").arg(bank).arg(name).arg(autosend).arg(port));
    dumpHex(data);
}

void QSpyOve::forcedChannel(int channel)
{
    dump(0, 0, NO_CHANNEL, "Forced channel", QString::number(channel));
}

void QSpyOve::forcedPort(int port)
{
    dump(0, 0, NO_CHANNEL, "Forced port", QString::number(port));
}

void QSpyOve::textEvent(int track, long tick, const QString& data)
{
    dump(tick, track, NO_CHANNEL, QString("Text "), data);
}

void QSpyOve::timeSigEvent(int bar, long tick, int num, int den)
{
    dumpStr(0, "Time Signature", QString("bar=%1, tick=%2, %3/%4").arg(bar).arg(tick).arg(num).arg(den));
}

void QSpyOve::keySigEvent(int bar, long tick, int alt)
{
    dumpStr(0, "Key Signature", QString("bar=%1, tick=%2, alt=%3").arg(bar).arg(tick).arg(alt));
}

void QSpyOve::tempoEvent(long tick, int tempo)
{
    double bpm = tempo / 100.0;
    dumpStr(tick, "Tempo", QString::number(bpm, 'f', 2));
}

void QSpyOve::trackPatch(int track, int channel, int patch)
{
    dump(0, track, QString::number(channel), "Track Patch", QString::number(patch));
}

void QSpyOve::trackVol(int track, int channel, int vol)
{
    Q_UNUSED(channel)
    dump(0, track, QString::number(channel), "Track Volume", QString::number(vol));
}

void QSpyOve::trackBank(int track, int channel, int bank)
{
    dump(0, track, QString::number(channel), "Track Bank", QString::number(bank));
}

void QSpyOve::chord(int track, long tick, const QString& name, const QByteArray& data)
{
    dump(tick, track, NO_CHANNEL, "Chord Diagram", name);
    dumpHex(data);
}

void QSpyOve::expression(int track, long tick, int code, const QString& text)
{
    dump(tick, track, NO_CHANNEL, "Expression", QString("text=%1 code=%2").arg(text).arg(code));
}

void QSpyOve::run(QString fileName)
{
    cout << "__ticks track ch event____________________ data____" << endl;
    m_engine->readFromFile(fileName);
}

int main(int argc, char *argv[])
{
    QSpyOve spy;
    CmdLineArgs args;
    args.setUsage("[options] file");
    args.addOption('v', "verbose", "Verbose output");
    args.addRequiredArgument("file", "Input OVE file name");
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
