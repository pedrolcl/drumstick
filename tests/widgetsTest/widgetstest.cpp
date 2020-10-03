#include <QObject>
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
    QCOMPARE(p, m_paletteList[PAL_SINGLE]);
    QCOMPARE(p.paletteId(), PAL_SINGLE);
    QCOMPARE(p.getNumColors(), 1);
    QCOMPARE(p.isHighLight(), true);
    QCOMPARE(p.isBackground(), false);
    QCOMPARE(p.isForeground(), false);
}

void WidgetsTest::testPaletteDouble()
{
    PianoPalette p(PAL_DOUBLE);
    QCOMPARE(p, m_paletteList[PAL_DOUBLE]);
    QCOMPARE(p.paletteId(), PAL_DOUBLE);
    QCOMPARE(p.getNumColors(), 2);
    QCOMPARE(p.isHighLight(), true);
    QCOMPARE(p.isBackground(), false);
    QCOMPARE(p.isForeground(), false);
}

void WidgetsTest::testPaletteChannels()
{
    PianoPalette p(PAL_CHANNELS);
    QCOMPARE(p, m_paletteList[PAL_CHANNELS]);
    QCOMPARE(p.paletteId(), PAL_CHANNELS);
    QCOMPARE(p.getNumColors(), 16);
    QCOMPARE(p.isHighLight(), true);
    QCOMPARE(p.isBackground(), false);
    QCOMPARE(p.isForeground(), false);
}

void WidgetsTest::testPaletteScale()
{
    PianoPalette p(PAL_SCALE);
    QCOMPARE(p, m_paletteList[PAL_SCALE]);
    QCOMPARE(p.paletteId(), PAL_SCALE);
    QCOMPARE(p.getNumColors(), 12);
    QCOMPARE(p.isHighLight(), false);
    QCOMPARE(p.isBackground(), true);
    QCOMPARE(p.isForeground(), false);
}

void WidgetsTest::testPaletteKeys()
{
    PianoPalette p(PAL_KEYS);
    QCOMPARE(p, m_paletteList[PAL_KEYS]);
    QCOMPARE(p.paletteId(), PAL_KEYS);
    QCOMPARE(p.getNumColors(), 2);
    QCOMPARE(p.isHighLight(), false);
    QCOMPARE(p.isBackground(), true);
    QCOMPARE(p.isForeground(), false);
}

void WidgetsTest::testPaletteFont()
{
    PianoPalette p(PAL_FONT);
    QCOMPARE(p, m_paletteList[PAL_FONT]);
    QCOMPARE(p.paletteId(), PAL_FONT);
    QCOMPARE(p.getNumColors(), 4);
    QCOMPARE(p.isHighLight(), false);
    QCOMPARE(p.isBackground(), false);
    QCOMPARE(p.isForeground(), true);
}

void WidgetsTest::testPaletteAssign()
{
    PianoPalette a(PAL_SINGLE);
    PianoPalette b(PAL_DOUBLE);
    a = b;
    QCOMPARE(a, b);
    QCOMPARE(a, m_paletteList[PAL_DOUBLE]);
    QCOMPARE(a.paletteId(), b.paletteId());
    QCOMPARE(a.getNumColors(), b.getNumColors());
}

void WidgetsTest::testPaletteUnchanged()
{
    PianoPalette p(PAL_KEYS);
    p.setColor(0, Qt::white);
    p.setColor(1, Qt::black);
    QCOMPARE(p, m_paletteList[PAL_KEYS]);
    QCOMPARE(p.paletteId(), PAL_KEYS);
    QCOMPARE(p.getNumColors(), 2);
}

void WidgetsTest::testPaletteChanged()
{
    PianoPalette p(PAL_KEYS);
    p.setColor(0, Qt::black);
    p.setColor(1, Qt::white);
    QVERIFY(p != m_paletteList[PAL_KEYS]);
    p.resetColors();
    QCOMPARE(p, m_paletteList[PAL_KEYS]);
}

QTEST_MAIN(WidgetsTest)

#include "widgetstest.moc"
