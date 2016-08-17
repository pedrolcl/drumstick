#include <QString>
#include <QtTest>
#include "alsaevent.h"

using namespace drumstick;

class AlsaTest1 : public QObject
{
    Q_OBJECT

public:
    AlsaTest1();

private Q_SLOTS:
    void testEvents();
};

AlsaTest1::AlsaTest1()
{
}

void AlsaTest1::testEvents()
{
    NoteEvent note(0, 60, 100, 120);
    QCOMPARE(note.getChannel(), 0);
    QCOMPARE(note.getKey(), 60);
    QCOMPARE(note.getVelocity(), 100);
    QCOMPARE(note.getDuration(), 120uL);

    NoteOnEvent noteOn(1, 60, 100);
    QCOMPARE(noteOn.getChannel(), 1);
    QCOMPARE(noteOn.getKey(), 60);
    QCOMPARE(noteOn.getVelocity(), 100);

    NoteOffEvent noteOff(2, 60, 0);
    QCOMPARE(noteOff.getChannel(), 2);
    QCOMPARE(noteOff.getKey(), 60);
    QCOMPARE(noteOff.getVelocity(), 0);

    ControllerEvent ctl(3, 33, 66);
    QCOMPARE(ctl.getChannel(), 3);
    QCOMPARE(ctl.getParam(), 33u);
    QCOMPARE(ctl.getValue(), 66);

    ProgramChangeEvent pgm(4, 123);
    QCOMPARE(pgm.getChannel(), 4);
    QCOMPARE(pgm.getValue(), 123);

    KeyPressEvent keyPress(5, 60, 124);
    QCOMPARE(keyPress.getChannel(), 5);
    QCOMPARE(keyPress.getKey(), 60);
    QCOMPARE(keyPress.getVelocity(), 124);

    ChanPressEvent chanPress(6, 111);
    QCOMPARE(chanPress.getChannel(), 6);
    QCOMPARE(chanPress.getValue(), 111);

    PitchBendEvent bender(7, 1234);
    QCOMPARE(bender.getChannel(), 7);
    QCOMPARE(bender.getValue(), 1234);

    QByteArray sysexData = QByteArray::fromHex("f04110421240007f0041f7");
    SysExEvent sysexEvent(sysexData);
    QCOMPARE(sysexEvent.getData(), sysexData.data());
    QCOMPARE(sysexEvent.getLength(), (unsigned) sysexData.length());

    QString text = "This can be a copyright, song name, instrument, lyric...";
    TextEvent textEvent(text, 3);
    QCOMPARE(textEvent.getText(), text);
    QCOMPARE(textEvent.getLength(), (unsigned) text.length());
}

QTEST_APPLESS_MAIN(AlsaTest1)

#include "alsatest1.moc"
