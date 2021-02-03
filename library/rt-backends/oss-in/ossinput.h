/*
    Drumstick RT (realtime MIDI In/Out)
    Copyright (C) 2009-2021 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef ossINPUT_H
#define ossINPUT_H

#include <QObject>
#include <QtPlugin>
#include <drumstick/rtmidiinput.h>
#include "ossinput_p.h"

namespace drumstick { namespace rt {

    class OSSInput : public MIDIInput
    {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "net.sourceforge.drumstick.rt.MIDIInput/2.0")
        Q_INTERFACES(drumstick::rt::MIDIInput)
    public:
        explicit OSSInput(QObject *parent = nullptr);
        virtual ~OSSInput();

        static const QString DEFAULT_PUBLIC_NAME;

        // MIDIInput interface
    public:
        virtual void initialize(QSettings* settings) override;
        virtual QString backendName() override;
        virtual QString publicName() override;
        virtual void setPublicName(QString name) override;
        virtual QList<MIDIConnection> connections(bool advanced) override;
        virtual void setExcludedConnections(QStringList conns) override;
        virtual void open(const MIDIConnection& name) override;
        virtual void close() override;
        virtual MIDIConnection currentConnection() override;

        virtual void setMIDIThruDevice(MIDIOutput *device) override;
        virtual void enableMIDIThru(bool enable) override;
        virtual bool isEnabledMIDIThru() override;
    private:
        OSSInputPrivate *d;
    };

}}

#endif // ossINPUT_H
