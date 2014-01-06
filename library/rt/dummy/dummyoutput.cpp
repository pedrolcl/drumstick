#include "dummyoutput.h"

namespace drumstick {
namespace rt {

QString DummyOutput::backendName()
{
    return QLatin1String("DUMMY");
}

QString DummyOutput::publicName()
{
    return QString();
}

void DummyOutput::setPublicName(QString name)
{
}

QStringList DummyOutput::connections(bool advanced)
{
    return QStringList();
}

void DummyOutput::setExcludedConnections(QStringList conns)
{
}

void DummyOutput::open(QString name)
{
}

void DummyOutput::close()
{
}

QString DummyOutput::currentConnection()
{
    return QString();
}

void DummyOutput::sendNoteOff(int chan, int note, int vel)
{
}

void DummyOutput::sendNoteOn(int chan, int note, int vel)
{
}

void DummyOutput::sendKeyPressure(int chan, int note, int value)
{
}

void DummyOutput::sendController(int chan, int control, int value)
{
}

void DummyOutput::sendProgram(int chan, int program)
{
}

void DummyOutput::sendChannelPressure(int chan, int value)
{
}

void DummyOutput::sendPitchBend(int chan, int value)
{
}

void DummyOutput::sendSysex(const QByteArray &data)
{
}

void DummyOutput::sendSystemMsg(const int status)
{
}

}}
