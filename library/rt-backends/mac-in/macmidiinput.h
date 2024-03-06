/*
    Drumstick RT Backend
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

#ifndef MACMIDIOBJECT_H
#define MACMIDIOBJECT_H

#include <drumstick/rtmidiinput.h>

namespace drumstick {
namespace rt {

    class MIDIOutput;
    class MacMIDIInputPrivate;

    class MacMIDIInput : public MIDIInput
    {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "net.sourceforge.drumstick.rt.MIDIInput/2.0")
        Q_INTERFACES(drumstick::rt::MIDIInput)
        Q_PROPERTY(QStringList diagnostics READ getDiagnostics)
        Q_PROPERTY(bool status READ getStatus);

    public:
        explicit MacMIDIInput(QObject *parent = nullptr);
        ~MacMIDIInput();

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

    private:
        MacMIDIInputPrivate* const d;
        QStringList getDiagnostics();
        bool getStatus();
    };

}}

#endif // MACMIDIOBJECT_H
