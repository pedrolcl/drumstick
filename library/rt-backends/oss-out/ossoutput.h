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

#ifndef ossOUTPUT_H
#define ossOUTPUT_H

#include <QObject>
#include <drumstick/rtmidioutput.h>

namespace drumstick { namespace rt {

    class OSSOutput : public MIDIOutput
    {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "net.sourceforge.drumstick.rt.MIDIOutput/2.0")
        Q_INTERFACES(drumstick::rt::MIDIOutput)
    public:
        explicit OSSOutput(QObject *parent = nullptr);
        virtual ~OSSOutput();

        static const QString DEFAULT_PUBLIC_NAME;

        // MIDIOutput interface
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

    public slots:
        virtual void sendNoteOff(int chan, int note, int vel) override;
        virtual void sendNoteOn(int chan, int note, int vel) override;
        virtual void sendKeyPressure(int chan, int note, int value) override;
        virtual void sendController(int chan, int control, int value) override;
        virtual void sendProgram(int chan, int program) override;
        virtual void sendChannelPressure(int chan, int value) override;
        virtual void sendPitchBend(int chan, int value) override;
        virtual void sendSysex(const QByteArray &data) override;
        virtual void sendSystemMsg(const int status) override;
    private:
        class OSSOutputPrivate;
        OSSOutputPrivate *d;
    };

}}

#endif // ossOUTPUT_H
