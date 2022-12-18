/*
    Standard RIFF MIDI File dump program
    Copyright (C) 2006-2022, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QObject>
#include <QString>
#include <QTextCodec>
#include <QTextStream>
#include <cstdlib>
#include "dumprmi.h"

DISABLE_WARNING_PUSH
DISABLE_WARNING_DEPRECATED_DECLARATIONS

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#define right Qt::right
#define left Qt::left
#define endl Qt::endl
#endif

QTextStream cout(stdout, QIODevice::WriteOnly);
QTextStream cerr(stderr, QIODevice::WriteOnly);

using namespace drumstick::File;

DumpRmid::DumpRmid():
    m_currentTrack(0), m_engine(nullptr), m_rc(0)
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    m_engine = new Rmidi(this);
    m_smf = new QSmf(this);
    m_riff = new Riff(this);    m_smf->setTextCodec(codec);

    connect(m_engine, &Rmidi::signalRiffInfo, this, &DumpRmid::infoHandler);
    connect(m_engine, &Rmidi::signalRiffData, this, &DumpRmid::dataHandler);

    connect(m_smf, &QSmf::signalSMFHeader, this, &DumpRmid::headerEvent);
    connect(m_smf, &QSmf::signalSMFTrackStart, this, &DumpRmid::trackStartEvent);
    connect(m_smf, &QSmf::signalSMFTrackEnd, this, &DumpRmid::trackEndEvent);
    connect(m_smf, &QSmf::signalSMFNoteOn, this, &DumpRmid::noteOnEvent);
    connect(m_smf, &QSmf::signalSMFNoteOff, this, &DumpRmid::noteOffEvent);
    connect(m_smf, &QSmf::signalSMFKeyPress, this, &DumpRmid::keyPressEvent);
    connect(m_smf, &QSmf::signalSMFCtlChange, this, &DumpRmid::ctlChangeEvent);
    connect(m_smf, &QSmf::signalSMFPitchBend, this, &DumpRmid::pitchBendEvent);
    connect(m_smf, &QSmf::signalSMFProgram, this, &DumpRmid::programEvent);
    connect(m_smf, &QSmf::signalSMFChanPress, this, &DumpRmid::chanPressEvent);
    connect(m_smf, &QSmf::signalSMFSysex, this, &DumpRmid::sysexEvent);
    connect(m_smf, &QSmf::signalSMFSeqSpecific, this, &DumpRmid::seqSpecificEvent);
    connect(m_smf, &QSmf::signalSMFMetaUnregistered, this, &DumpRmid::metaMiscEvent);
    connect(m_smf, &QSmf::signalSMFSequenceNum, this, &DumpRmid::seqNum);
    connect(m_smf, &QSmf::signalSMFforcedChannel, this, &DumpRmid::forcedChannel);
    connect(m_smf, &QSmf::signalSMFforcedPort, this, &DumpRmid::forcedPort);
    connect(m_smf, &QSmf::signalSMFText, this, &DumpRmid::textEvent);
    connect(m_smf, &QSmf::signalSMFendOfTrack, this, &DumpRmid::endOfTrackEvent);
    connect(m_smf, &QSmf::signalSMFTimeSig, this, &DumpRmid::timeSigEvent);
    connect(m_smf, &QSmf::signalSMFSmpte, this, &DumpRmid::smpteEvent);
    connect(m_smf, &QSmf::signalSMFKeySig, this, &DumpRmid::keySigEvent);
    connect(m_smf, &QSmf::signalSMFTempo, this, &DumpRmid::tempoEvent);
    connect(m_smf, &QSmf::signalSMFError, this, &DumpRmid::errorHandler);

    connect(m_riff, &Riff::signalDLS, this, &DumpRmid::processDLS);
    connect(m_riff, &Riff::signalInstrument, this, &DumpRmid::processInstrument);
    connect(m_riff, &Riff::signalPercussion, this, &DumpRmid::processPercussion);

    cout.setRealNumberNotation(QTextStream::FixedNotation);
    cout.setRealNumberPrecision(4);
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
    cout.setCodec(codec);
#else
    cout.setEncoding(QStringConverter::Utf8);
#endif
}

void DumpRmid::dump(const QString& chan, const QString& event,
                   const QString& data)
{
    cout << right << qSetFieldWidth(7) << m_smf->getCurrentTime();
    cout << right << qSetFieldWidth(10) << m_smf->getRealTime() / 1600.0;
    cout << qSetFieldWidth(3) << chan;
    cout << qSetFieldWidth(0) << left << " ";
    cout << qSetFieldWidth(15) << event;
    cout << qSetFieldWidth(0) << " " << data << endl;
}

void DumpRmid::dumpStr(const QString& event, const QString& data)
{
    cout << right << qSetFieldWidth(7) << m_smf->getCurrentTime();
    cout << right << qSetFieldWidth(10) << m_smf->getRealTime() / 1600.0;
    cout << qSetFieldWidth(3) << "--";
    cout << qSetFieldWidth(0) << left << " ";
    cout << qSetFieldWidth(15) << event;
    cout << qSetFieldWidth(0) << " " << data << endl;
}

void DumpRmid::headerEvent(int format, int ntrks, int division)
{
    dumpStr("SMF Header", QString("Format=%1, Tracks=%2, Division=%3")
            .arg(format).arg(ntrks).arg(division));
}

void DumpRmid::trackStartEvent()
{
    m_currentTrack++;
    dumpStr("Track", QString("Start: %1").arg(m_currentTrack));
}

void DumpRmid::trackEndEvent()
{
    dumpStr("Track", QString("End: %1").arg(m_currentTrack));
}

void DumpRmid::endOfTrackEvent()
{
    dumpStr("Meta Event", "End Of Track");
}

void DumpRmid::noteOnEvent(int chan, int pitch, int vol)
{
    dump(QString("%1").arg(chan, 2), "Note On", QString("%1, %2").arg(pitch).arg(vol));
}

void DumpRmid::noteOffEvent(int chan, int pitch, int vol)
{
    dump(QString("%1").arg(chan, 2), "Note Off", QString("%1, %2").arg(pitch).arg(vol));
}

void DumpRmid::keyPressEvent(int chan, int pitch, int press)
{
    dump(QString("%1").arg(chan, 2), "Key Pressure", QString("%1, %2").arg(pitch).arg(press));
}

void DumpRmid::ctlChangeEvent(int chan, int ctl, int value)
{
    dump(QString("%1").arg(chan, 2), "Control Change", QString("%1, %2").arg(ctl).arg(value));
}

void DumpRmid::pitchBendEvent(int chan, int value)
{
    dump(QString("%1").arg(chan, 2), "Pitch Bend", QString::number(value));
}

void DumpRmid::programEvent(int chan, int patch)
{
    dump(QString("%1").arg(chan, 2), "Program Change", QString::number(patch));
}

void DumpRmid::chanPressEvent(int chan, int press)
{
    dump(QString("%1").arg(chan, 2), "Chan.Pressure", QString::number(press));
}

void DumpRmid::sysexEvent(const QByteArray& data)
{
    int j;
    QString s;
    for (j = 0; j < data.count(); ++j)
        s.append(QString("%1 ").arg(data[j] & 0xff, 2, 16));
    dumpStr("SysEx", s);
}

void DumpRmid::seqSpecificEvent(const QByteArray& data)
{
    int j;
    QString s;
    for (j = 0; j < data.count(); ++j)
        s.append(QString("%1 ").arg(data[j] & 0xff, 2, 16));
    dumpStr("Seq. specific", s);
}

void DumpRmid::metaMiscEvent(int typ, const QByteArray& data)
{
    int j;
    QString s = QString("type=%1 ").arg(typ);
    for (j = 0; j < data.count(); ++j)
        s.append(QString("%1 ").arg(data[j] & 0xff, 2, 16));
    dumpStr("Meta (unreg.)", s);
}

void DumpRmid::seqNum(int seq)
{
    dump("--", "Sequence num.", QString::number(seq));
}

void DumpRmid::forcedChannel(int channel)
{
    dump("--", "Forced channel", QString::number(channel));
}

void DumpRmid::forcedPort(int port)
{
    dump("--", "Forced port", QString::number(port));
}

void DumpRmid::textEvent(int typ, const QString& data)
{
    dumpStr(QString("Text (%1)").arg(typ), data);
}

void DumpRmid::smpteEvent(int b0, int b1, int b2, int b3, int b4)
{
    dump("--", "SMPTE", QString("%1, %2, %3, %4, %5").arg(b0).arg(b1).arg(b2).arg(b3).arg(b4));
}

void DumpRmid::timeSigEvent(int b0, int b1, int b2, int b3)
{
    dump("--", "Time Signature", QString("%1, %2, %3, %4").arg(b0).arg(b1).arg(b2).arg(b3));
}

void DumpRmid::keySigEvent(int b0, int b1)
{
    dump("--", "Key Signature", QString("%1, %2").arg(b0).arg(b1));
}

void DumpRmid::tempoEvent(int tempo)
{
    dump("--", "Tempo", QString::number(tempo));
}

void DumpRmid::errorHandler(const QString& errorStr)
{
    m_rc++;
    cerr << "*** Warning! " << errorStr
         << " at file offset " << m_smf->getFilePos()
         << endl;
}

void DumpRmid::infoHandler(const QString &infoType, const QByteArray &data)
{    
    QString info = QString::fromLatin1(data);
    if (m_extract) {
        cout << infoType << ": " << info << endl;
    } else {
        dump("--", "Info", QString("%1: %2").arg(infoType, info));
    }
}

void DumpRmid::extractFileData(const QString &fileSuffix, const QByteArray &data)
{
    QFileInfo finfo(m_fileName);
    QString outfile = QDir::current().absoluteFilePath(finfo.baseName() + fileSuffix);
    QFile file(outfile);
    file.open(QFile::WriteOnly);
    file.write(data);
    file.close();
}

void DumpRmid::dataHandler(const QString &dataType, const QByteArray &data)
{
    if (m_extract) {
        if (dataType == "RMID") {
            extractFileData(QLatin1String(".mid"), data);
        } else if (dataType == "DLS") {
            extractFileData(QLatin1String(".dls"), data);
        }
    } else {
        if (dataType == "RMID") {
            QDataStream ds(data);
            m_smf->readFromStream(&ds);
        } else if (dataType == "DLS") {
            cout << " __bank ____PC name_____________________" << endl;
            QDataStream ds(data);
            m_riff->readFromStream(&ds);
        }
    }
}

void DumpRmid::processDLS(QString name, QString version, QString copyright)
{
    cout << "DLS: " << name;
    if (!version.isEmpty()) {
        cout << " version: " << version;
    }
    if (!copyright.isNull()) {
        cout << " copyright: " << copyright;
    }
    cout << endl;
}

void DumpRmid::processInstrument(int bank, int pc, QString name)
{
    cout << right << qSetFieldWidth(7) << bank;
    cout << right << qSetFieldWidth(7) << pc;
    cout << left << qSetFieldWidth(0) << " " << name << endl;
}

void DumpRmid::processPercussion(int bank, int pc, QString name)
{
    cout << right << qSetFieldWidth(7) << bank;
    cout << right << qSetFieldWidth(7) << pc;
    cout << left << qSetFieldWidth(0) << " " << name << endl;
}

void DumpRmid::run(QString fileName)
{
    m_fileName = fileName;
    m_currentTrack = 0;
    if (!m_extract) {
        cout << "__ticks __seconds ch event__________ data____" << endl;
    }
    m_engine->readFromFile(fileName);
}

int DumpRmid::numErrors()
{
    return m_rc;
}

void DumpRmid::setExtract(bool enable)
{
    m_extract = enable;
}

DISABLE_WARNING_POP
