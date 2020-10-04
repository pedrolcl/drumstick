#include <QString>
#include <QDataStream>
#include <QByteArray>
#include <QtTest>
#include <drumstick/qwrk.h>

using namespace drumstick::File;

class FileTest2 : public QObject
{
    Q_OBJECT

public:
    explicit FileTest2(QObject* parent = 0);

    static const char test_wrk[];
    static const int test_wrk_len;

public slots:
    void fileHeader(int verh, int verl);
    void trackHeader(const QString& name1, const QString& name2,
                     int trackno, int channel, int pitch,
                     int velocity, int port,
                     bool selected, bool muted, bool loop);
    void timeBase(int timebase);
    void noteEvent(int track, long time, int chan, int pitch, int vol, int dur);
    void timeSigEvent(int bar, int num, int den);
    void keySigEvent(int bar, int alt);
    void tempoEvent(long time, int tempo);
    void errorHandler(const QString& errorStr);
    void newTrackHeader(const QString& name,
                        int trackno, int channel, int pitch,
                        int velocity, int port,
                        bool selected, bool muted, bool loop);

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testCaseReadWrkFile();

private:
    QWrk *m_engine;
    int m_timeBase;
    int m_numNotes;
    int m_lastNote;
    int m_tracks;
    int m_lastKeySig;
    float m_lastTempo;
    QByteArray m_testData;
    QString m_fileVersion;
    QString m_lastTimeSig;
    QString m_lastError;
};

FileTest2::FileTest2(QObject* parent): QObject(parent),
    m_engine(nullptr),
    m_timeBase(0),
    m_numNotes(0),
    m_lastNote(0),
    m_tracks(0),
    m_lastKeySig(0),
    m_lastTempo(0)
{
    m_engine = new QWrk(this);
    m_engine->setTextCodec(QTextCodec::codecForName("UTF-8"));

    connect(m_engine, &QWrk::signalWRKError, this, &FileTest2::errorHandler);
    connect(m_engine, &QWrk::signalWRKHeader, this, &FileTest2::fileHeader);
    connect(m_engine, &QWrk::signalWRKTrack, this, &FileTest2::trackHeader);
    connect(m_engine, &QWrk::signalWRKTimeBase, this, &FileTest2::timeBase);
    connect(m_engine, &QWrk::signalWRKNote, this, &FileTest2::noteEvent);
    connect(m_engine, &QWrk::signalWRKTimeSig, this, &FileTest2::timeSigEvent);
    connect(m_engine, &QWrk::signalWRKKeySig, this, &FileTest2::keySigEvent);
    connect(m_engine, &QWrk::signalWRKTempo, this, &FileTest2::tempoEvent);
    connect(m_engine, &QWrk::signalWRKNewTrack, this, &FileTest2::newTrackHeader);
}

const char FileTest2::test_wrk[] = {
    '\x43','\x41','\x4b','\x45','\x57','\x41','\x4c','\x4b','\x1a','\x00','\x02','\x0a','\x02','\x00','\x00','\x00',
    '\xc0','\x00','\x03','\x59','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
    '\x03','\x00','\x00','\x00','\x00','\x05','\x00','\x00','\x00','\x01','\x01','\x00','\x01','\x00','\x03','\x00',
    '\x00','\x01','\x00','\x00','\x00','\x00','\x00','\x01','\x01','\x00','\xff','\xff','\x01','\x7b','\x00','\x00',
    '\x00','\x01','\x02','\x03','\x04','\x05','\x06','\x07','\x08','\x09','\x0a','\x0b','\x0c','\x0d','\x0e','\x0f',
    '\x01','\x01','\x20','\x40','\x80','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
    '\x40','\x02','\x00','\x00','\xfe','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
    '\x16','\x16','\x00','\x00','\x00','\x03','\x00','\x06','\x4d','\x43','\x49','\x43','\x6d','\x64','\x01','\x04',
    '\x57','\x61','\x76','\x65','\x02','\x04','\x54','\x65','\x78','\x74','\x03','\x0f','\x14','\x00','\x00','\x00',
    '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\xe0','\x2e','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x00','\x05','\x0e','\x00','\x00','\x00','\x01','\x00','\x00','\x00','\x00','\x00','\x01',
    '\x00','\x04','\x02','\x00','\x00','\x00','\x00','\x17','\x07','\x00','\x00','\x00','\x01','\x00','\x01','\x00',
    '\x04','\x02','\x00','\x0b','\x06','\x00','\x00','\x00','\x1e','\x00','\x00','\x00','\x00','\x00','\x11','\x0e',
    '\x00','\x00','\x00','\x00','\x00','\x09','\x00','\x25','\x00','\x6e','\x00','\x01','\x00','\x00','\x00','\x00',
    '\x00','\x10','\x0c','\x00','\x00','\x00','\x02','\x00','\x00','\x09','\x00','\x7f','\xff','\xff','\x00','\x00',
    '\x00','\x00','\x01','\x09','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x09','\x00','\x7f','\x00','\x00',
    '\x02','\x2c','\x00','\x00','\x00','\x00','\x00','\x05','\x00','\x00','\x00','\x00','\x90','\x58','\x64','\xc0',
    '\x00','\x00','\x00','\x00','\x90','\x25','\x64','\xc0','\x00','\xc0','\x00','\x00','\x90','\x25','\x64','\xc0',
    '\x00','\x80','\x01','\x00','\x90','\x25','\x64','\xc0','\x00','\x40','\x02','\x00','\x90','\x25','\x64','\xc0',
    '\x00','\x13','\x06','\x00','\x00','\x00','\x00','\x00','\x7f','\x00','\x40','\x00','\x19','\x42','\x00','\x00',
    '\x00','\x10','\x00','\x00','\x00','\x07','\x3f','\x01','\x00','\x07','\x3f','\x02','\x00','\x07','\x3f','\x03',
    '\x00','\x07','\x3f','\x04','\x00','\x07','\x3f','\x05','\x00','\x07','\x3f','\x06','\x00','\x07','\x3f','\x07',
    '\x00','\x07','\x3f','\x08','\x00','\x07','\x3f','\x09','\x00','\x07','\x3f','\x0a','\x00','\x07','\x3f','\x0b',
    '\x00','\x07','\x3f','\x0c','\x00','\x07','\x3f','\x0d','\x00','\x07','\x3f','\x0e','\x00','\x07','\x3f','\x0f',
    '\x00','\x07','\x3f','\xff'
};
const int FileTest2::test_wrk_len = sizeof(test_wrk); //388

void FileTest2::fileHeader(int verh, int verl)
{
    m_fileVersion = QString("%1.%2").arg(verh).arg(verl);
}

void FileTest2::errorHandler(const QString& errorStr)
{
    m_lastError = errorStr;
    qWarning() << errorStr;
}

void FileTest2::trackHeader( const QString& name1, const QString& name2,
                           int trackno, int channel, int pitch,
                           int velocity, int port,
                           bool selected, bool muted, bool loop )
{
    Q_UNUSED(name1)
    Q_UNUSED(name2)
    Q_UNUSED(trackno)
    Q_UNUSED(channel)
    Q_UNUSED(pitch)
    Q_UNUSED(velocity)
    Q_UNUSED(port)
    Q_UNUSED(selected)
    Q_UNUSED(muted)
    Q_UNUSED(loop)
    m_tracks++;
}

void FileTest2::timeBase(int timebase)
{
    m_timeBase = timebase;
}

void FileTest2::noteEvent(int track, long time, int chan, int pitch, int vol, int dur)
{
    Q_UNUSED(track)
    Q_UNUSED(time)
    Q_UNUSED(chan)
    Q_UNUSED(vol)
    Q_UNUSED(dur)
    m_numNotes++;
    m_lastNote = pitch;
}

void FileTest2::timeSigEvent(int bar, int num, int den)
{
    Q_UNUSED(bar)
    m_lastTimeSig = QString("%1/%2").arg(num).arg(den);
}

void FileTest2::keySigEvent(int bar, int alt)
{
    Q_UNUSED(bar)
    m_lastKeySig = alt;
}

void FileTest2::tempoEvent(long time, int tempo)
{
    Q_UNUSED(time)
    double bpm = tempo / 100.0;
    m_lastTempo = bpm;
}

void FileTest2::newTrackHeader( const QString& name,
                              int trackno, int channel, int pitch,
                              int velocity, int port,
                              bool selected, bool muted, bool loop )
{
    Q_UNUSED(name)
    Q_UNUSED(trackno)
    Q_UNUSED(channel)
    Q_UNUSED(pitch)
    Q_UNUSED(velocity)
    Q_UNUSED(port)
    Q_UNUSED(selected)
    Q_UNUSED(muted)
    Q_UNUSED(loop)
    m_tracks++;
}

void FileTest2::initTestCase()
{
    m_testData = QByteArray::fromRawData(test_wrk, test_wrk_len);
}

void FileTest2::cleanupTestCase()
{
    m_testData.clear();
}

void FileTest2::testCaseReadWrkFile()
{
    QDataStream stream(&m_testData,  QIODevice::ReadWrite);
    m_engine->readFromStream(&stream);
    if (!m_lastError.isEmpty()) {
        QFAIL(m_lastError.toLocal8Bit());
    }
    QCOMPARE(m_fileVersion, "2.0");
    QCOMPARE(m_timeBase, 192);
    QCOMPARE(m_tracks, 1);
    QCOMPARE(m_lastTempo, 120);
    QCOMPARE(m_lastTimeSig, "4/4");
    QCOMPARE(m_lastKeySig, 0);
    QCOMPARE(m_numNotes, 5);
    QCOMPARE(m_lastNote, 37);
}

QTEST_APPLESS_MAIN(FileTest2)

#include "filetest2.moc"
