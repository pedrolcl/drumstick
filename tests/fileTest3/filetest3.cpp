/*
    Copyright (C) 2008-2024, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This file is part of the Drumstick project, see https://sf.net/p/drumstick

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; If not, see <http://www.gnu.org/licenses/>.
*/

#include <QByteArray>
#include <QDataStream>
#include <QString>
#include <QtTest>
#include <QTextCodec>
#include <drumstick/qsmf.h>
#include <drumstick/rmid.h>

DISABLE_WARNING_PUSH
DISABLE_WARNING_DEPRECATED_DECLARATIONS

using namespace drumstick::File;

class FileTest3 : public QObject
{
    Q_OBJECT
public:
    explicit FileTest3(QObject* parent = nullptr);

    static const char test_rmi[];
    static const int test_rmi_len;
    static const int FORMAT;
    static const int TRACKS;
    static const int DIVISION;
    static const int TEMPO;
    static const QList<int> NOTES;

public Q_SLOTS:
    void dataHandler(const QString& dataType, const QByteArray& data);

    void errorHandler(const QString& errorStr);
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
    void textEvent(int typ, const QString& data);
    void timeSigEvent(int b0, int b1, int b2, int b3);
    void keySigEvent(int b0, int b1);
    void tempoEvent(int tempo);

private Q_SLOTS:
    void testCaseReadRmidi();
    void initTestCase();
    void cleanupTestCase();

private:
    Rmidi *m_rmidi;
    QSmf *m_engine;
    int m_numNoteOn;
    int m_lastNoteOn;
    int m_numNoteOff;
    int m_lastNoteOff;
    int m_lastKeyPress;
    int m_currentTrack;
    int m_endOfTrack;
    int m_lastCtl;
    int m_lastProgram;
    int m_lastChanPress;
    int m_lastPitchBend;
    int m_lastTempo;
    int m_format;
    int m_ntrks;
    int m_division;
    QByteArray m_testData;
    QByteArray m_lastSysex;
    QString m_lastError;
    QString m_trackEnd;
    QString m_lastTextEvent;
    QString m_lastTimeSig;
    QString m_lastKeySig;
};

FileTest3::FileTest3(QObject* parent): QObject(parent),
    m_rmidi(nullptr),
    m_engine(nullptr),
    m_numNoteOn(0),
    m_lastNoteOn(0),
    m_numNoteOff(0),
    m_lastNoteOff(0),
    m_lastKeyPress(0),
    m_currentTrack(0),
    m_endOfTrack(0),
    m_lastCtl(0),
    m_lastProgram(0),
    m_lastChanPress(0),
    m_lastPitchBend(0),
    m_lastTempo(0),
    m_format(0),
    m_ntrks(0),
    m_division(0)
{
    m_rmidi = new Rmidi(this);
    connect(m_rmidi, &Rmidi::signalRiffData, this, &FileTest3::dataHandler);

    m_engine = new QSmf(this);
    m_engine->setTextCodec(QTextCodec::codecForName("UTF-8"));

    connect(m_engine, &QSmf::signalSMFError, this, &FileTest3::errorHandler);
    connect(m_engine, &QSmf::signalSMFHeader, this, &FileTest3::headerEvent);
    connect(m_engine, &QSmf::signalSMFTrackStart, this, &FileTest3::trackStartEvent);
    connect(m_engine, &QSmf::signalSMFTrackEnd, this, &FileTest3::trackEndEvent);
    connect(m_engine, &QSmf::signalSMFNoteOn, this, &FileTest3::noteOnEvent);
    connect(m_engine, &QSmf::signalSMFNoteOff, this, &FileTest3::noteOffEvent);
    connect(m_engine, &QSmf::signalSMFKeyPress, this, &FileTest3::keyPressEvent);
    connect(m_engine, &QSmf::signalSMFCtlChange, this, &FileTest3::ctlChangeEvent);
    connect(m_engine, &QSmf::signalSMFPitchBend, this, &FileTest3::pitchBendEvent);
    connect(m_engine, &QSmf::signalSMFProgram, this, &FileTest3::programEvent);
    connect(m_engine, &QSmf::signalSMFChanPress, this, &FileTest3::chanPressEvent);
    connect(m_engine, &QSmf::signalSMFSysex, this, &FileTest3::sysexEvent);
    connect(m_engine, &QSmf::signalSMFText, this, &FileTest3::textEvent);
    connect(m_engine, &QSmf::signalSMFendOfTrack, this, &FileTest3::endOfTrackEvent);
    connect(m_engine, &QSmf::signalSMFTimeSig, this, &FileTest3::timeSigEvent);
    connect(m_engine, &QSmf::signalSMFKeySig, this, &FileTest3::keySigEvent);
    connect(m_engine, &QSmf::signalSMFTempo, this, &FileTest3::tempoEvent);
}

const char FileTest3::test_rmi[] = {
  '\x52', '\x49', '\x46', '\x46', '\x7c', '\x00', '\x00', '\x00', '\x52', '\x4d', '\x49', '\x44',
  '\x64', '\x61', '\x74', '\x61', '\x70', '\x00', '\x00', '\x00', '\x4d', '\x54', '\x68', '\x64',
  '\x00', '\x00', '\x00', '\x06', '\x00', '\x00', '\x00', '\x01', '\x03', '\xc0', '\x4d', '\x54',
  '\x72', '\x6b', '\x00', '\x00', '\x00', '\x5a', '\x00', '\xff', '\x58', '\x04', '\x04', '\x02',
  '\x18', '\x08', '\x00', '\xff', '\x59', '\x02', '\x00', '\x00', '\x00', '\xff', '\x51', '\x03',
  '\x07', '\xa1', '\x20', '\x00', '\xb0', '\x07', '\x65', '\x00', '\x90', '\x3c', '\x64', '\x83',
  '\x60', '\x3c', '\x00', '\x00', '\x3e', '\x64', '\x83', '\x60', '\x3e', '\x00', '\x00', '\x40',
  '\x64', '\x83', '\x60', '\x40', '\x00', '\x00', '\x41', '\x64', '\x83', '\x60', '\x41', '\x00',
  '\x00', '\x43', '\x64', '\x83', '\x60', '\x43', '\x00', '\x00', '\x45', '\x64', '\x83', '\x60',
  '\x45', '\x00', '\x00', '\x47', '\x64', '\x83', '\x60', '\x47', '\x00', '\x00', '\x48', '\x64',
  '\x83', '\x60', '\x48', '\x00', '\x00', '\xb0', '\x07', '\x65', '\x00', '\xff', '\x2f', '\x00'
};
const int FileTest3::test_rmi_len = sizeof(test_rmi); //132;
const QList<int> FileTest3::NOTES = { 60, 62, 64, 65, 67, 69, 71, 72 };
const int FileTest3::FORMAT = 0;
const int FileTest3::TRACKS = 1;
const int FileTest3::DIVISION = 960;
const int FileTest3::TEMPO = 120;

void FileTest3::dataHandler(const QString &dataType, const QByteArray &data)
{
    if (dataType == "RMID") {
        QDataStream ds(data);
        m_engine->readFromStream(&ds);
    }
}

void FileTest3::errorHandler(const QString& errorStr)
{
    m_lastError = errorStr;
    qWarning() << Q_FUNC_INFO << errorStr;
}

void FileTest3::headerEvent(int format, int ntrks, int division)
{
    m_format = format;
    m_ntrks = ntrks;
    m_division = division;
}

void FileTest3::trackStartEvent()
{
    m_currentTrack++;
}

void FileTest3::trackEndEvent()
{
    m_trackEnd = QString("End: %1").arg(m_currentTrack);
    //qDebug() << Q_FUNC_INFO << m_trackEnd;
}

void FileTest3::endOfTrackEvent()
{
    m_endOfTrack++;
}

void FileTest3::noteOnEvent(int , int pitch, int vel)
{
    Q_UNUSED(vel)
    m_numNoteOn++;
    m_lastNoteOn = pitch;
    //qDebug() << Q_FUNC_INFO << pitch << vel;
}

void FileTest3::noteOffEvent(int , int pitch, int vel)
{
    Q_UNUSED(vel)
    m_numNoteOff++;
    m_lastNoteOff = pitch;
    //qDebug() << Q_FUNC_INFO << pitch << vel;
}

void FileTest3::keyPressEvent(int , int pitch, int )
{
    m_lastKeyPress = pitch;
}

void FileTest3::ctlChangeEvent(int , int ctl, int )
{
    m_lastCtl = ctl;
}

void FileTest3::pitchBendEvent(int , int value)
{
    m_lastPitchBend = value;
}

void FileTest3::programEvent(int , int patch)
{
    m_lastProgram = patch;
}

void FileTest3::chanPressEvent(int , int press)
{
    m_lastChanPress = press;
}

void FileTest3::sysexEvent(const QByteArray& data)
{
    m_lastSysex = data;
}

void FileTest3::textEvent(int , const QString& data)
{
    m_lastTextEvent = data;
}

void FileTest3::timeSigEvent(int b0, int b1, int b2, int b3)
{
    m_lastTimeSig = QString("%1, %2, %3, %4").arg(b0).arg(b1).arg(b2).arg(b3);
    //qDebug() << Q_FUNC_INFO << m_lastTimeSig;
}

void FileTest3::keySigEvent(int b0, int b1)
{
    m_lastKeySig = QString("%1, %2").arg(b0).arg(b1);
    //qDebug() << Q_FUNC_INFO << m_lastKeySig;
}

void FileTest3::tempoEvent(int tempo)
{
    m_lastTempo = static_cast<int>( 6e7 / tempo );
    //qDebug() << Q_FUNC_INFO << m_lastTempo;
}

void FileTest3::initTestCase()
{
    m_testData = QByteArray::fromRawData(test_rmi, test_rmi_len);
}

void FileTest3::cleanupTestCase()
{
    m_testData.clear();
}

void FileTest3::testCaseReadRmidi()
{
    QDataStream stream(&m_testData,  QIODevice::ReadOnly);
    m_rmidi->readFromStream(&stream);
    if (!m_lastError.isEmpty()) {
        QFAIL(m_lastError.toLocal8Bit());
    }
    QCOMPARE(m_format, FORMAT);
    QCOMPARE(m_ntrks, TRACKS);
    QCOMPARE(m_division, DIVISION);
    QCOMPARE(m_engine->getFileFormat(), FORMAT);
    QCOMPARE(m_engine->getTracks(), TRACKS);
    QCOMPARE(m_engine->getDivision(), DIVISION);
    QCOMPARE(m_lastTempo, TEMPO);
    QCOMPARE(m_numNoteOn, NOTES.length() * 2);
    QCOMPARE(m_numNoteOff, 0); // no notes off
    QCOMPARE(m_lastNoteOn, NOTES.last());
    QCOMPARE(m_lastNoteOff, 0); // no notes off
    QCOMPARE(m_currentTrack, TRACKS);
    QCOMPARE(m_endOfTrack, TRACKS);
}

QTEST_APPLESS_MAIN(FileTest3)

#include "filetest3.moc"

DISABLE_WARNING_POP
