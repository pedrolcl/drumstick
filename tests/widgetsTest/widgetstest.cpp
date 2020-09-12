#include <QString>
#include <QDataStream>
#include <QByteArray>
#include <QtTest>
#include <drumstick/pianopalette.h>

using namespace drumstick::widgets;

class WidgetsTest : public QObject
{
    Q_OBJECT
public:
    WidgetsTest();

private Q_SLOTS:
    void testPaletteSingle();
    void testPaletteDouble();
    void testPaletteChannels();
    void testPaletteScale();
    void testPaletteKeys();
    void testPaletteFont();

    void testPaletteAssign();
    void testPaletteUnchanged();
    void testPaletteChanged();

private:
    QList<PianoPalette> m_paletteList {
        PianoPalette(PAL_SINGLE),
        PianoPalette(PAL_DOUBLE),
        PianoPalette(PAL_CHANNELS),
        PianoPalette(PAL_SCALE),
        PianoPalette(PAL_KEYS),
        PianoPalette(PAL_FONT),
    };
};

WidgetsTest::WidgetsTest()
{
//    for( PianoPalette& pal : m_paletteList) {
//        pal.paletteId();
//    }
}

void WidgetsTest::testPaletteSingle()
{
    PianoPalette p(PAL_SINGLE);
    QCOMPARE(p, m_paletteList[0]);
    QCOMPARE(p.paletteId(), PAL_SINGLE);
    QCOMPARE(p.getNumColors(), 1);
}

void WidgetsTest::testPaletteDouble()
{
    PianoPalette p(PAL_DOUBLE);
    QCOMPARE(p, m_paletteList[1]);
    QCOMPARE(p.paletteId(), PAL_DOUBLE);
    QCOMPARE(p.getNumColors(), 2);
}

void WidgetsTest::testPaletteChannels()
{
    PianoPalette p(PAL_CHANNELS);
    QCOMPARE(p, m_paletteList[2]);
    QCOMPARE(p.paletteId(), PAL_CHANNELS);
    QCOMPARE(p.getNumColors(), 16);
}

void WidgetsTest::testPaletteScale()
{
    PianoPalette p(PAL_SCALE);
    QCOMPARE(p, m_paletteList[3]);
    QCOMPARE(p.paletteId(), PAL_SCALE);
    QCOMPARE(p.getNumColors(), 12);
}

void WidgetsTest::testPaletteKeys()
{
    PianoPalette p(PAL_KEYS);
    QCOMPARE(p, m_paletteList[4]);
    QCOMPARE(p.paletteId(), PAL_KEYS);
    QCOMPARE(p.getNumColors(), 2);
}

void WidgetsTest::testPaletteFont()
{
    PianoPalette p(PAL_FONT);
    QCOMPARE(p, m_paletteList[5]);
    QCOMPARE(p.paletteId(), PAL_FONT);
    QCOMPARE(p.getNumColors(), 4);
}

void WidgetsTest::testPaletteAssign()
{
    PianoPalette a(PAL_SINGLE);
    PianoPalette b(PAL_DOUBLE);
    a = b;
    QCOMPARE(a, b);
    QCOMPARE(a, m_paletteList[1]);
    QCOMPARE(a.paletteId(), b.paletteId());
    QCOMPARE(a.getNumColors(), b.getNumColors());
}

void WidgetsTest::testPaletteUnchanged()
{
    PianoPalette p(PAL_KEYS);
    p.setColor(0, Qt::white);
    p.setColor(1, Qt::black);
    //QEXPECT_FAIL("", "Colors not matching", Continue);
    QCOMPARE(p, m_paletteList[4]);
    //QEXPECT_FAIL("", "Ids not matching", Continue);
    QCOMPARE(p.paletteId(), PAL_KEYS);
    //QEXPECT_FAIL("", "Number of Colors not matching", Continue);
    QCOMPARE(p.getNumColors(), 2);
}

void WidgetsTest::testPaletteChanged()
{
    PianoPalette p(PAL_KEYS);
    p.setColor(0, Qt::black);
    p.setColor(1, Qt::white);
    QVERIFY(p != m_paletteList[4]);
    p.resetColors();
    QCOMPARE(p, m_paletteList[4]);
}

QTEST_MAIN(WidgetsTest)

#include "widgetstest.moc"
