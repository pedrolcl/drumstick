#ifndef DUMMYOUTPUT_H
#define DUMMYOUTPUT_H

#include <QObject>
#include "rtmidioutput.h"

namespace drumstick {
namespace rt {

    class DummyOutput : public MIDIOutput
    {
        Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        Q_PLUGIN_METADATA(IID "net.sourceforge.drumstick.rt.MIDIOutput")
#endif
        Q_INTERFACES(drumstick::rt::MIDIOutput)
    public:
        DummyOutput(QObject *parent = 0) : MIDIOutput(parent) {}
        virtual ~DummyOutput() {}

        // MIDIOutput interface
    public:
        virtual QString backendName();
        virtual QString publicName();
        virtual void setPublicName(QString name);
        virtual QStringList connections(bool advanced);
        virtual void setExcludedConnections(QStringList conns);
        virtual void open(QString name);
        virtual void close();
        virtual QString currentConnection();

    public slots:
        virtual void sendNoteOff(int chan, int note, int vel);
        virtual void sendNoteOn(int chan, int note, int vel);
        virtual void sendKeyPressure(int chan, int note, int value);
        virtual void sendController(int chan, int control, int value);
        virtual void sendProgram(int chan, int program);
        virtual void sendChannelPressure(int chan, int value);
        virtual void sendPitchBend(int chan, int value);
        virtual void sendSysex(const QByteArray &data);
        virtual void sendSystemMsg(const int status);
    };
}}

#endif // DUMMYOUTPUT_H
