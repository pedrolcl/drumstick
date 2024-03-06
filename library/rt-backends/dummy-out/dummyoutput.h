/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2024 Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DUMMYOUTPUT_H
#define DUMMYOUTPUT_H

#include <QObject>
#include <drumstick/rtmidioutput.h>

namespace drumstick {
namespace rt {

    class DummyOutput : public MIDIOutput
    {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "net.sourceforge.drumstick.rt.MIDIOutput/2.0")
        Q_INTERFACES(drumstick::rt::MIDIOutput)
        Q_PROPERTY(QStringList diagnostics READ getDiagnostics)
        Q_PROPERTY(QString libversion READ getLibVersion)
        Q_PROPERTY(bool status READ getStatus)
        Q_PROPERTY(bool isconfigurable READ getConfigurable)

    public:
        explicit DummyOutput(QObject *parent = nullptr) : MIDIOutput(parent) {}
        virtual ~DummyOutput() = default;

        // MIDIOutput interface
    public:
        virtual void initialize(QSettings* settings);
        virtual QString backendName();
        virtual QString publicName();
        virtual void setPublicName(QString name);
        virtual QList<MIDIConnection> connections(bool advanced);
        virtual void setExcludedConnections(QStringList conns);
        virtual void open(const MIDIConnection& name);
        virtual void close();
        virtual MIDIConnection currentConnection();

    public Q_SLOTS:
        virtual void sendNoteOff(int chan, int note, int vel);
        virtual void sendNoteOn(int chan, int note, int vel);
        virtual void sendKeyPressure(int chan, int note, int value);
        virtual void sendController(int chan, int control, int value);
        virtual void sendProgram(int chan, int program);
        virtual void sendChannelPressure(int chan, int value);
        virtual void sendPitchBend(int chan, int value);
        virtual void sendSysex(const QByteArray &data);
        virtual void sendSystemMsg(const int status);

        bool configure(QWidget *parent);

    private:
        QStringList getDiagnostics();
        QString getLibVersion();
        bool getStatus();
        bool getConfigurable();
    };

}}

#endif // DUMMYOUTPUT_H
