#include <QString>
#include <QtTest>

class DrumstickTest : public QObject
{
    Q_OBJECT

public:
    DrumstickTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testCase1();
};

DrumstickTest::DrumstickTest()
{
}

void DrumstickTest::initTestCase()
{
}

void DrumstickTest::cleanupTestCase()
{
}

void DrumstickTest::testCase1()
{
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(DrumstickTest)

#include "drumsticktest.moc"
