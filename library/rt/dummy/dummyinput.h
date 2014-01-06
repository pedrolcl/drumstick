#ifndef DUMMYINPUT_H
#define DUMMYINPUT_H

#include <QObject>
#include <QtPlugin>
#include "rtmidiinput.h"

namespace drumstick {
namespace rt {

    class DummyInput : public MIDIInput
    {
        Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        Q_PLUGIN_METADATA(IID "net.sourceforge.drumstick.rt.MIDIInput")
#endif
        Q_INTERFACES(drumstick::rt::MIDIInput)
    public:
        DummyInput(QObject *parent = 0) : MIDIInput(parent) {}
        virtual ~DummyInput() {}

        // MIDIInput interface
    public:
        virtual QString backendName();
        virtual QString publicName();
        virtual void setPublicName(QString name);
        virtual QStringList connections(bool advanced);
        virtual void setExcludedConnections(QStringList conns);
        virtual void open(QString name);
        virtual void close();
        virtual QString currentConnection();

        virtual void setMIDIThruDevice(MIDIOutput *device);
        virtual void enableMIDIThru(bool enable);
        virtual bool isEnabledMIDIThru();
    };

}}

#endif // DUMMYINPUT_H
