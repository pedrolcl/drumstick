#include <QString>
#include <QStringList>
#include <QtTest>
#include "rtmidiinput.h"
#include "rtmidioutput.h"
#include "backendmanager.h"

using namespace drumstick::rt;

class RtTest : public QObject
{
    Q_OBJECT

public:
    RtTest();

private Q_SLOTS:
    void testRT();
};

RtTest::RtTest()
{
}

void RtTest::testRT()
{
    QList<MIDIInput*> inputsList;
    QList<MIDIOutput*> outputsList;
    BackendManager man;

    QStringList paths = man.defaultPaths();
    QVERIFY2(paths.length() > 0, "Plugins path is empty");

    inputsList = man.availableInputs();
    QVERIFY2(inputsList.length() > 0, "There aren't input backends");
    foreach(MIDIInput* input, inputsList) {
        QCOMPARE(input->backendName().isEmpty(), false );
        QCOMPARE(input->publicName().isEmpty(), false );
        QStringList conns = input->connections();
        QVERIFY2(conns.length() > 0, "Backend without any connection");
        QStringList avconns = input->connections(true);
        QVERIFY2(avconns.length() > 0, "Backend without any advanced connection");
        QVERIFY2(avconns.length() >= conns.length(), "unexpected connections number");
    }

    outputsList = man.availableOutputs();
    QVERIFY2(outputsList.length() > 0, "There aren't output backends");
    foreach(MIDIOutput* output, outputsList) {
        QCOMPARE(output->backendName().isEmpty(), false );
        QCOMPARE(output->publicName().isEmpty(), false );
        QStringList conns = output->connections();
        QVERIFY2(conns.length() > 0, "Backend without any connection");
        QStringList avconns = output->connections(true);
        QVERIFY2(avconns.length() > 0, "Backend without any advanced connection");
        QVERIFY2(avconns.length() >= conns.length(), "unexpected connections number");
    }
}

QTEST_MAIN(RtTest)

#include "rttest.moc"
