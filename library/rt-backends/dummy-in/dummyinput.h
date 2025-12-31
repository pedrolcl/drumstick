/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2025 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef DUMMYINPUT_H
#define DUMMYINPUT_H

#include <QObject>
#include <drumstick/rtmidiinput.h>

namespace drumstick {
namespace rt {

    class DummyInput : public MIDIInput
    {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "net.sourceforge.drumstick.rt.MIDIInput/2.0")
        Q_INTERFACES(drumstick::rt::MIDIInput)
        Q_PROPERTY(QStringList diagnostics READ getDiagnostics)
        Q_PROPERTY(QString libversion READ getLibVersion)
        Q_PROPERTY(bool status READ getStatus)
        Q_PROPERTY(bool isconfigurable READ getConfigurable)

    public:
        explicit DummyInput(QObject *parent = nullptr) : MIDIInput(parent) {}
        virtual ~DummyInput() = default;

        // MIDIInput interface
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

        virtual void setMIDIThruDevice(MIDIOutput *device);
        virtual void enableMIDIThru(bool enable);
        virtual bool isEnabledMIDIThru();

    public Q_SLOTS:
        bool configure(QWidget *parent);

    private:
        QStringList getDiagnostics();
        QString getLibVersion();
        bool getStatus();
        bool getConfigurable();
    };

}}

#endif // DUMMYINPUT_H
