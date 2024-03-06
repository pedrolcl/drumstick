/*
    Sonivox EAS Synthesizer for Qt applications
    Copyright (C) 2016-2024, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SYNTHCONTROLLER_H
#define SYNTHCONTROLLER_H

#include <QObject>
#include <QThread>
#include <QWaitCondition>
#include <drumstick/rtmidioutput.h>
#include "synthrenderer.h"

namespace drumstick {
namespace rt {

    class SynthController : public MIDIOutput
    {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "net.sourceforge.drumstick.rt.MIDIOutput/2.0")
        Q_INTERFACES(drumstick::rt::MIDIOutput)
        Q_PROPERTY(QStringList diagnostics READ getDiagnostics)
        Q_PROPERTY(bool status READ getStatus)
        Q_PROPERTY(QString libversion READ getLibVersion)
        Q_PROPERTY(QString soundfont READ getSoundFont)

    public:
        explicit SynthController(QObject *parent = nullptr);
        virtual ~SynthController();

        void start();
        void stop();

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

    public Q_SLOTS:
        virtual void sendNoteOff(int chan, int note, int vel) override;
        virtual void sendNoteOn(int chan, int note, int vel) override;
        virtual void sendKeyPressure(int chan, int note, int value) override;
        virtual void sendController(int chan, int control, int value) override;
        virtual void sendProgram(int chan, int program) override;
        virtual void sendChannelPressure(int chan, int value) override;
        virtual void sendPitchBend(int chan, int value) override;
        virtual void sendSysex(const QByteArray &data) override;
        virtual void sendSystemMsg(const int status) override;

        void writeSettings(QSettings *settings);

    private:
        QThread m_renderingThread;
        SynthRenderer *m_renderer;
        QWaitCondition m_rendering;

    private:
        QStringList getDiagnostics();
        bool getStatus();
        QString getLibVersion();
        QString getSoundFont();
    };

}}
#endif // SYNTHCONTROLLER_H
