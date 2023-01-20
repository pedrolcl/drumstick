/*
    Copyright (C) 2008-2023, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

DISABLE_WARNING_PUSH
DISABLE_WARNING_DEPRECATED_DECLARATIONS

using namespace drumstick::File;

class FileTest1 : public QObject
{
    Q_OBJECT
public:
    explicit FileTest1(QObject* parent = nullptr);

    static const char test_mid[];
    static const int test_mid_len;
    static const int FORMAT;
    static const int TRACKS;
    static const int DIVISION;
    static const int TEMPO;
    static const QString COPYRIGHT;
    static const QByteArray GSRESET;
    static const QList<int> NOTES;

public Q_SLOTS:
    void errorHandler(const QString& errorStr);
    void trackHandler(int track);
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
    void testCaseWriteSmf();
    void testCaseReadSmf();
    void initTestCase();
    void cleanupTestCase();

private:
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
    QByteArray m_testData;
    QByteArray m_lastSysex;
    QString m_lastError;
    QString m_header;
    QString m_trackEnd;
    QString m_lastTextEvent;
    QString m_lastTimeSig;
    QString m_lastKeySig;
};

FileTest1::FileTest1(QObject* parent): QObject(parent),
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
    m_lastTempo(0)
{
    m_engine = new QSmf(this);
    m_engine->setTextCodec(QTextCodec::codecForName("UTF-8"));

    connect(m_engine, &QSmf::signalSMFError, this, &FileTest1::errorHandler);
    connect(m_engine, &QSmf::signalSMFWriteTrack, this, &FileTest1::trackHandler);

    connect(m_engine, &QSmf::signalSMFHeader, this, &FileTest1::headerEvent);
    connect(m_engine, &QSmf::signalSMFTrackStart, this, &FileTest1::trackStartEvent);
    connect(m_engine, &QSmf::signalSMFTrackEnd, this, &FileTest1::trackEndEvent);
    connect(m_engine, &QSmf::signalSMFNoteOn, this, &FileTest1::noteOnEvent);
    connect(m_engine, &QSmf::signalSMFNoteOff, this, &FileTest1::noteOffEvent);
    connect(m_engine, &QSmf::signalSMFKeyPress, this, &FileTest1::keyPressEvent);
    connect(m_engine, &QSmf::signalSMFCtlChange, this, &FileTest1::ctlChangeEvent);
    connect(m_engine, &QSmf::signalSMFPitchBend, this, &FileTest1::pitchBendEvent);
    connect(m_engine, &QSmf::signalSMFProgram, this, &FileTest1::programEvent);
    connect(m_engine, &QSmf::signalSMFChanPress, this, &FileTest1::chanPressEvent);
    connect(m_engine, &QSmf::signalSMFSysex, this, &FileTest1::sysexEvent);
    connect(m_engine, &QSmf::signalSMFText, this, &FileTest1::textEvent);
    connect(m_engine, &QSmf::signalSMFendOfTrack, this, &FileTest1::endOfTrackEvent);
    connect(m_engine, &QSmf::signalSMFTimeSig, this, &FileTest1::timeSigEvent);
    connect(m_engine, &QSmf::signalSMFKeySig, this, &FileTest1::keySigEvent);
    connect(m_engine, &QSmf::signalSMFTempo, this, &FileTest1::tempoEvent);
}

const char FileTest1::test_mid[] = {
  '\x4d','\x54','\x68','\x64','\x00','\x00','\x00','\x06','\x00','\x00','\x00','\x01',
  '\x00','\x78','\x4d','\x54','\x72','\x6b','\x00','\x00','\x00','\x99','\x00','\xff',
  '\x02','\x2f','\x43','\x6f','\x70','\x79','\x72','\x69','\x67','\x68','\x74','\x20',
  '\x28','\x43','\x29','\x20','\x32','\x30','\x30','\x36','\x2d','\x32','\x30','\x32',
  '\x33','\x20','\x50','\x65','\x64','\x72','\x6f','\x20','\x4c','\xc3','\xb3','\x70',
  '\x65','\x7a','\x2d','\x43','\x61','\x62','\x61','\x6e','\x69','\x6c','\x6c','\x61',
  '\x73','\x00','\xff','\x51','\x03','\x09','\x27','\xc0','\x00','\xff','\x58','\x04',
  '\x03','\x02','\x24','\x08','\x00','\xff','\x59','\x02','\x02','\x00','\x00','\xf0',
  '\x0a','\x41','\x10','\x42','\x12','\x40','\x00','\x7f','\x00','\x41','\xf7','\x00',
  '\x90','\x3c','\x78','\x3c','\x80','\x3c','\x00','\x00','\x90','\x3e','\x78','\x3c',
  '\x80','\x3e','\x00','\x00','\x90','\x40','\x78','\x3c','\x80','\x40','\x00','\x00',
  '\x90','\x41','\x78','\x3c','\x80','\x41','\x00','\x00','\x90','\x43','\x78','\x3c',
  '\x80','\x43','\x00','\x00','\x90','\x45','\x78','\x3c','\x80','\x45','\x00','\x00',
  '\x90','\x47','\x78','\x3c','\x80','\x47','\x00','\x00','\x90','\x48','\x78','\x3c',
  '\x80','\x48','\x00','\x00','\xff','\x2f','\x00'
};
const int FileTest1::test_mid_len = sizeof(test_mid); //175;
const QString FileTest1::COPYRIGHT = u8"Copyright (C) 2006-2023 Pedro López-Cabanillas";
const QByteArray FileTest1::GSRESET = QByteArrayLiteral( "f04110421240007f0041f7" );
const QList<int> FileTest1::NOTES = { 60, 62, 64, 65, 67, 69, 71, 72 };
const int FileTest1::FORMAT = 0;
const int FileTest1::TRACKS = 1;
const int FileTest1::DIVISION = 120;
const int FileTest1::TEMPO = 100;

void FileTest1::errorHandler(const QString& errorStr)
{
    m_lastError = errorStr;
    qWarning() << errorStr;
}

void FileTest1::trackHandler(int )
{
    int i;
    // Text event
    m_engine->writeMetaEvent(0, copyright_notice, COPYRIGHT);
    m_engine->writeBpmTempo(0, TEMPO);
    m_engine->writeTimeSignature(0, 3, 2, 36, 8);  // ts = 3/4
    m_engine->writeKeySignature(0, 2, major_mode); // D major (2 sharps)
    // system exclusive event
    QByteArray gsreset = QByteArray::fromHex( GSRESET );
    m_engine->writeMidiEvent(0, system_exclusive, long(gsreset.size()), gsreset.data());
    // some note events
    for(i = 0; i < NOTES.length(); ++i)
    {
        m_engine->writeMidiEvent(0,  note_on,  0, NOTES[i], 120);
        m_engine->writeMidiEvent(60, note_off, 0, NOTES[i], 0);
    }
    // final event
    m_engine->writeMetaEvent(0, end_of_track);
}

void FileTest1::headerEvent(int format, int ntrks, int division)
{
    m_header = QString("Format=%1, Tracks=%2, Division=%3").arg(format).arg(ntrks).arg(division);
}

void FileTest1::trackStartEvent()
{
    m_currentTrack++;
}

void FileTest1::trackEndEvent()
{
    m_trackEnd = QString("End: %1").arg(m_currentTrack);
}

void FileTest1::endOfTrackEvent()
{
    m_endOfTrack++;
}

void FileTest1::noteOnEvent(int , int pitch, int )
{
    m_numNoteOn++;
    m_lastNoteOn = pitch;
}

void FileTest1::noteOffEvent(int , int pitch, int )
{
    m_numNoteOff++;
    m_lastNoteOff = pitch;
}

void FileTest1::keyPressEvent(int , int pitch, int )
{
    m_lastKeyPress = pitch;
}

void FileTest1::ctlChangeEvent(int , int ctl, int )
{
    m_lastCtl = ctl;
}

void FileTest1::pitchBendEvent(int , int value)
{
    m_lastPitchBend = value;
}

void FileTest1::programEvent(int , int patch)
{
    m_lastProgram = patch;
}

void FileTest1::chanPressEvent(int , int press)
{
    m_lastChanPress = press;
}

void FileTest1::sysexEvent(const QByteArray& data)
{
    m_lastSysex = data;
}

void FileTest1::textEvent(int , const QString& data)
{
    m_lastTextEvent = data;
}

void FileTest1::timeSigEvent(int b0, int b1, int b2, int b3)
{
    m_lastTimeSig = QString("%1, %2, %3, %4").arg(b0).arg(b1).arg(b2).arg(b3);
}

void FileTest1::keySigEvent(int b0, int b1)
{
    m_lastKeySig = QString("%1, %2").arg(b0).arg(b1);
}

void FileTest1::tempoEvent(int tempo)
{
    m_lastTempo = static_cast<int>( 6e7 / tempo );
}

void FileTest1::initTestCase()
{
    m_testData = QByteArray::fromRawData(test_mid, test_mid_len);
}

void FileTest1::cleanupTestCase()
{
    m_testData.clear();
}

void FileTest1::testCaseWriteSmf()
{
    QByteArray data;
    QDataStream stream(&data,  QIODevice::ReadWrite);
    m_engine->setDivision(DIVISION);
    m_engine->setFileFormat(FORMAT);
    m_engine->setTracks(TRACKS);
    m_engine->writeToStream(&stream);
    if (!m_lastError.isEmpty()) {
        QFAIL(m_lastError.toLocal8Bit());
    }
    QCOMPARE(data, m_testData);
}

void FileTest1::testCaseReadSmf()
{
    QDataStream stream(&m_testData,  QIODevice::ReadWrite);
    m_engine->readFromStream(&stream);
    if (!m_lastError.isEmpty()) {
        QFAIL(m_lastError.toLocal8Bit());
    }
    QCOMPARE(m_engine->getFileFormat(), FORMAT);
    QCOMPARE(m_engine->getDivision(), DIVISION);
    QCOMPARE(m_engine->getTracks(), TRACKS);
    QCOMPARE(m_lastTempo, TEMPO);
    QCOMPARE(m_lastTextEvent, COPYRIGHT);
    QCOMPARE(m_lastSysex, QByteArray::fromHex(GSRESET));
    QCOMPARE(m_numNoteOn, NOTES.length());
    QCOMPARE(m_numNoteOff, NOTES.length());
    QCOMPARE(m_lastNoteOn, NOTES.last());
    QCOMPARE(m_lastNoteOff, NOTES.last());
    QCOMPARE(m_currentTrack, TRACKS);
    QCOMPARE(m_endOfTrack, TRACKS);
}

QTEST_APPLESS_MAIN(FileTest1)

#include "filetest1.moc"

DISABLE_WARNING_POP
