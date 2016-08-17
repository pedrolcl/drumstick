#include <QString>
#include <QDataStream>
#include <QByteArray>
#include <QtTest>
#include "qsmf.h"

const unsigned char test_mid[] = {
  0x4d, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x78, 0x4d, 0x54, 0x72, 0x6b, 0x00, 0x00, 0x00, 0x99, 0x00, 0xff,
  0x02, 0x2f, 0x43, 0x6f, 0x70, 0x79, 0x72, 0x69, 0x67, 0x68, 0x74, 0x20,
  0x28, 0x43, 0x29, 0x20, 0x32, 0x30, 0x30, 0x36, 0x2d, 0x32, 0x30, 0x31,
  0x36, 0x20, 0x50, 0x65, 0x64, 0x72, 0x6f, 0x20, 0x4c, 0xc3, 0xb3, 0x70,
  0x65, 0x7a, 0x2d, 0x43, 0x61, 0x62, 0x61, 0x6e, 0x69, 0x6c, 0x6c, 0x61,
  0x73, 0x00, 0xff, 0x51, 0x03, 0x09, 0x27, 0xc0, 0x00, 0xff, 0x58, 0x04,
  0x03, 0x02, 0x24, 0x08, 0x00, 0xff, 0x59, 0x02, 0x02, 0x00, 0x00, 0xf0,
  0x0a, 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7f, 0x00, 0x41, 0xf7, 0x00,
  0x90, 0x3c, 0x78, 0x3c, 0x80, 0x3c, 0x00, 0x00, 0x90, 0x3e, 0x78, 0x3c,
  0x80, 0x3e, 0x00, 0x00, 0x90, 0x40, 0x78, 0x3c, 0x80, 0x40, 0x00, 0x00,
  0x90, 0x41, 0x78, 0x3c, 0x80, 0x41, 0x00, 0x00, 0x90, 0x43, 0x78, 0x3c,
  0x80, 0x43, 0x00, 0x00, 0x90, 0x45, 0x78, 0x3c, 0x80, 0x45, 0x00, 0x00,
  0x90, 0x47, 0x78, 0x3c, 0x80, 0x47, 0x00, 0x00, 0x90, 0x48, 0x78, 0x3c,
  0x80, 0x48, 0x00, 0x00, 0xff, 0x2f, 0x00
};
const int test_mid_len = 175;

const int FORMAT( 0 );
const int TRACKS( 1 );
const int DIVISION( 120 );
const int TEMPO( 100 );
const QString COPYRIGHT( "Copyright (C) 2006-2016 Pedro López-Cabanillas" );
const QByteArray GSRESET( "f04110421240007f0041f7" );
const QList<int> NOTES({ 60, 62, 64, 65, 67, 69, 71, 72 });

using namespace drumstick;

class FileTest : public QObject
{
    Q_OBJECT
public:
    FileTest();

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

private:
    QSmf *m_engine;
    QDataStream *m_stream;
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
    QByteArray m_data;
    QByteArray m_expected;
    QByteArray m_lastSysex;
    QString m_lastError;
    QString m_header;
    QString m_trackEnd;
    QString m_lastTextEvent;
    QString m_lastTimeSig;
    QString m_lastKeySig;
};

FileTest::FileTest():
    m_engine(0),
    m_stream(0),
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
    m_stream = new QDataStream(&m_data, QIODevice::ReadWrite);
    m_expected = QByteArray::fromRawData((const char *) test_mid, test_mid_len);

    connect(m_engine, SIGNAL(signalSMFError(const QString&)), this, SLOT(errorHandler(const QString&)));
    connect(m_engine, SIGNAL(signalSMFWriteTrack(int)), this, SLOT(trackHandler(int)));

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
    connect(m_engine, SIGNAL(signalSMFText(int,const QString&)), this, SLOT(textEvent(int,const QString&)));
    connect(m_engine, SIGNAL(signalSMFendOfTrack()), this, SLOT(endOfTrackEvent()));
    connect(m_engine, SIGNAL(signalSMFTimeSig(int,int,int,int)), this, SLOT(timeSigEvent(int,int,int,int)));
    connect(m_engine, SIGNAL(signalSMFKeySig(int,int)), this, SLOT(keySigEvent(int,int)));
    connect(m_engine, SIGNAL(signalSMFTempo(int)), this, SLOT(tempoEvent(int)));
}

void FileTest::errorHandler(const QString& errorStr)
{
    m_lastError = errorStr;
    qWarning() << errorStr;
}

void FileTest::trackHandler(int )
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

void FileTest::headerEvent(int format, int ntrks, int division)
{
    m_header = QString("Format=%1, Tracks=%2, Division=%3")
                .arg(format).arg(ntrks).arg(division);
}

void FileTest::trackStartEvent()
{
    m_currentTrack++;
}

void FileTest::trackEndEvent()
{
    m_trackEnd = QString("End: %1").arg(m_currentTrack);
}

void FileTest::endOfTrackEvent()
{
    m_endOfTrack++;
}

void FileTest::noteOnEvent(int , int pitch, int )
{
    m_numNoteOn++;
    m_lastNoteOn = pitch;
}

void FileTest::noteOffEvent(int , int pitch, int )
{
    m_numNoteOff++;
    m_lastNoteOff = pitch;
}

void FileTest::keyPressEvent(int , int pitch, int )
{
    m_lastKeyPress = pitch;
}

void FileTest::ctlChangeEvent(int , int ctl, int )
{
    m_lastCtl = ctl;
}

void FileTest::pitchBendEvent(int , int value)
{
    m_lastPitchBend = value;
}

void FileTest::programEvent(int , int patch)
{
    m_lastProgram = patch;
}

void FileTest::chanPressEvent(int , int press)
{
    m_lastChanPress = press;
}

void FileTest::sysexEvent(const QByteArray& data)
{
    m_lastSysex = data;
}

void FileTest::textEvent(int , const QString& data)
{
    m_lastTextEvent = data;
}

void FileTest::timeSigEvent(int b0, int b1, int b2, int b3)
{
    m_lastTimeSig = QString("%1, %2, %3, %4").arg(b0).arg(b1).arg(b2).arg(b3);
}

void FileTest::keySigEvent(int b0, int b1)
{
    m_lastKeySig = QString("%1, %2").arg(b0).arg(b1);
}

void FileTest::tempoEvent(int tempo)
{
    m_lastTempo = 6e7 / tempo;
}

void FileTest::testCaseWriteSmf()
{
    m_data.clear();
    m_engine->setDivision(DIVISION);
    m_engine->setFileFormat(FORMAT);
    m_engine->setTracks(TRACKS);
    m_engine->writeToStream(m_stream);
    if (!m_lastError.isEmpty()) {
        QFAIL(m_lastError.toLocal8Bit());
    }
    QCOMPARE(m_data, m_expected);
}

void FileTest::testCaseReadSmf()
{
    m_stream->device()->reset();
    m_engine->readFromStream(m_stream);
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

QTEST_APPLESS_MAIN(FileTest)

#include "filetest.moc"
