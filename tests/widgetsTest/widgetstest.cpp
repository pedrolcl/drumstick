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

    void testPaletteAssign();
    void testPaletteChange();

private:
    QList<PianoPalette> m_paletteList {
        PianoPalette(1, PAL_SINGLE),
        PianoPalette(2, PAL_DOUBLE),
        PianoPalette(16, PAL_CHANNELS),
        PianoPalette(12, PAL_SCALE),
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
    PianoPalette p(1, PAL_SINGLE);
    QCOMPARE(p, m_paletteList[0]);
    QCOMPARE(p.paletteId(), PAL_SINGLE);
    QCOMPARE(p.getNumColors(), 1);
}

void WidgetsTest::testPaletteDouble()
{
    PianoPalette p(2, PAL_DOUBLE);
    QCOMPARE(p, m_paletteList[1]);
    QCOMPARE(p.paletteId(), PAL_DOUBLE);
    QCOMPARE(p.getNumColors(), 2);
}

void WidgetsTest::testPaletteChannels()
{
    PianoPalette p(16, PAL_CHANNELS);
    QCOMPARE(p, m_paletteList[2]);
    QCOMPARE(p.paletteId(), PAL_CHANNELS);
    QCOMPARE(p.getNumColors(), 16);
}

void WidgetsTest::testPaletteScale()
{
    PianoPalette p(12, PAL_SCALE);
    QCOMPARE(p, m_paletteList[3]);
    QCOMPARE(p.paletteId(), PAL_SCALE);
    QCOMPARE(p.getNumColors(), 12);
}

void WidgetsTest::testPaletteAssign()
{
    PianoPalette a(1, PAL_SINGLE);
    PianoPalette b(2, PAL_DOUBLE);
    a = b;
    QCOMPARE(a, b);
    QCOMPARE(a, m_paletteList[1]);
    QCOMPARE(a.paletteId(), b.paletteId());
    QCOMPARE(a.getNumColors(), b.getNumColors());
}

void WidgetsTest::testPaletteChange()
{
    PianoPalette p(2, PAL_DOUBLE);
    p.setColor(0, Qt::white);
    p.setColor(1, Qt::black);
    QEXPECT_FAIL("", "Colors not matching", Continue);
    QCOMPARE(p, m_paletteList[1]);
    QEXPECT_FAIL("", "Ids not matching", Continue);
    QCOMPARE(p.paletteId(), PAL_SINGLE);
    QEXPECT_FAIL("", "Number of Colors not matching", Continue);
    QCOMPARE(p.getNumColors(), 1);
}

QTEST_MAIN(WidgetsTest)

#include "widgetstest.moc"
