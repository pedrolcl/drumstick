#include <QString>
#include <QStringList>
#include <QtTest>
#include <drumstick/rtmidiinput.h>
#include <drumstick/rtmidioutput.h>
#include <drumstick/backendmanager.h>

#if defined(LINUX_BACKEND)
Q_IMPORT_PLUGIN(ALSAMIDIInput)
Q_IMPORT_PLUGIN(ALSAMIDIOutput)
Q_IMPORT_PLUGIN(SynthController)
#endif

#if defined(MAC_BACKEND)
Q_IMPORT_PLUGIN(MacMIDIInput)
Q_IMPORT_PLUGIN(MacMIDIOutput)
Q_IMPORT_PLUGIN(MacSynthOutput)
#endif

#if defined(WIN_BACKEND)
Q_IMPORT_PLUGIN(WinMIDIInput)
Q_IMPORT_PLUGIN(WinMIDIOutput)
#endif

#if defined(NET_BACKEND)
Q_IMPORT_PLUGIN(NetMIDIInput)
Q_IMPORT_PLUGIN(NetMIDIOutput)
#endif

#if defined(DUMMY_BACKEND)
Q_IMPORT_PLUGIN(DummyInput)
Q_IMPORT_PLUGIN(DummyOutput)
#endif

#if defined(FLUIDSYNTH_BACKEND)
Q_IMPORT_PLUGIN(SynthOutput)
#endif

#if defined(OSS_BACKEND)
Q_IMPORT_PLUGIN(OSSInput)
Q_IMPORT_PLUGIN(OSSOutput)
#endif

using namespace drumstick::rt;

class RtTest : public QObject
{
    Q_OBJECT

public:
    RtTest();

private:
    QString joinConns(QList<MIDIConnection> conns);

private Q_SLOTS:
    void testRT();
};

RtTest::RtTest()
{
}

void RtTest::testRT()
{
    QSettings settings;
    QList<MIDIInput*> inputsList;
    QList<MIDIOutput*> outputsList;
    BackendManager man;

    man.refresh(&settings);
    QStringList paths = man.defaultPaths();
#if !defined(DRUMSTICK_STATIC)
    QVERIFY2(paths.length() > 0, "Plugins path is empty");
    foreach(const QString& p, paths) {
        qDebug() << "path:" << p;
    }
#endif

    inputsList = man.availableInputs();
    QVERIFY2(inputsList.length() > 0, "There aren't input backends");
    foreach(MIDIInput* input, inputsList) {
        QList<MIDIConnection> conns = input->connections();
        qDebug() << "input:" << input->backendName() << input->publicName();
        qDebug() << " connections:" << (conns.isEmpty() ? "none" : joinConns(conns));
        QCOMPARE(input->backendName().isEmpty(), false );
        QCOMPARE(input->publicName().isEmpty(), false );
        /*QVERIFY2(conns.length() > 0, "Backend without any connection");
        QStringList avconns = input->connections(true);
        QVERIFY2(avconns.length() > 0, "Backend without any advanced connection");
        QVERIFY2(avconns.length() >= conns.length(), "unexpected connections number");*/
    }

    outputsList = man.availableOutputs();
    QVERIFY2(outputsList.length() > 0, "There aren't output backends");
    foreach(MIDIOutput* output, outputsList) {
        QList<MIDIConnection> conns = output->connections();
        qDebug() << "output:" << output->backendName() << output->publicName();
        qDebug() << " connections:" << (conns.isEmpty() ? "none" : joinConns(conns));
        /*QVERIFY2(conns.length() > 0, "Backend without any connection");
        QStringList avconns = output->connections(true);
        QVERIFY2(avconns.length() > 0, "Backend without any advanced connection");
        QVERIFY2(avconns.length() >= conns.length(), "unexpected connections number");*/
    }
}

QString RtTest::joinConns(QList<MIDIConnection> conns)
{
    QString res;
    for(const MIDIConnection& c : conns) {
        res += c.first + ", ";
    }
    return res;
}

QTEST_GUILESS_MAIN(RtTest)

#include "rttest.moc"
