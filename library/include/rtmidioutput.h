/*
    Drumstick MIDI realtime input-output
    Copyright (C) 2009-2015 Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef MIDIOUTPUT_H
#define MIDIOUTPUT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QtPlugin>
#include <QSettings>

#define MIDI_CHANNELS               16
#define MIDI_GM_DRUM_CHANNEL        (10-1)
#define MIDI_CTL_MSB_MAIN_VOLUME    0x07
#define MIDI_CTL_ALL_SOUNDS_OFF     0x78
#define MIDI_CTL_ALL_NOTES_OFF      0x7b
#define MIDI_CTL_RESET_CONTROLLERS  0x79

#define MIDI_STATUS_NOTEOFF         0x80
#define MIDI_STATUS_NOTEON          0x90
#define MIDI_STATUS_KEYPRESURE      0xa0
#define MIDI_STATUS_CONTROLCHANGE   0xb0
#define MIDI_STATUS_PROGRAMCHANGE   0xc0
#define MIDI_STATUS_CHANNELPRESSURE 0xd0
#define MIDI_STATUS_PITCHBEND       0xe0
#define MIDI_STATUS_SYSEX           0xf0
#define MIDI_STATUS_ENDSYSEX        0xf7
#define MIDI_STATUS_REALTIME        0xf8

#define MIDI_STATUS_MASK            0xf0
#define MIDI_CHANNEL_MASK           0x0f

#define MIDI_COMMON_QTRFRAME        0xF1
#define MIDI_COMMON_SONGPP          0xF2
#define MIDI_COMMON_SONSELECT       0xF3
#define MIDI_COMMON_TUNEREQ         0xF6

#define MIDI_REALTIME_CLOCK         0xF8
#define MIDI_REALTIME_START         0xFA
#define MIDI_REALTIME_CONTINUE      0xFB
#define MIDI_REALTIME_STOP          0xFC
#define MIDI_REALTIME_SENSING       0xFE
#define MIDI_REALTIME_RESET         0xFF

#define MIDI_LSB(x) (x % 0x80)
#define MIDI_MSB(x) (x / 0x80)

namespace drumstick {
namespace rt {

    /**
     * MIDI OUT interface
     */
    class MIDIOutput : public QObject
    {
        Q_OBJECT

    public:
        /**
         * @brief MIDIOutput constructor
         * @param parent
         */
        explicit MIDIOutput(QObject *parent = 0) : QObject(parent) {}
        /**
         * @brief ~MIDIOutput destructor
         */
        virtual ~MIDIOutput() {}
        /**
         * @brief initialize
         * @param settings
         */
        virtual void initialize(QSettings* settings) = 0;
        /**
         * @brief backendName
         * @return plugin name
         */
        virtual QString backendName() = 0;
        /**
         * @brief publicName
         * @return MIDI port name
         */
        virtual QString publicName() = 0;
        /**
         * @brief setPublicName
         * @param name MIDI port name
         */
        virtual void setPublicName(QString name) = 0;
        /**
         * @brief connections
         * @return list of available MIDI ports
         */
        virtual QStringList connections(bool advanced = false) = 0;
        /**
         * @brief setExcludedConnections
         * @param conns
         */
        virtual void setExcludedConnections(QStringList conns) = 0;
        /**
         * @brief open the MIDI port by name
         * @param name
         */
        virtual void open(QString name) = 0;
        /**
         * @brief close the MIDI port
         */
        virtual void close() = 0;
        /**
         * @brief currentConnection
         * @return name of the current connection if it is opened
         */
        virtual QString currentConnection() = 0;

    public Q_SLOTS:
        /**
         * @brief sendNoteOff 0x8
         * @param chan
         * @param note
         * @param vel
         */
        virtual void sendNoteOff(int chan, int note, int vel) = 0;

        /**
         * @brief sendNoteOn 0x9
         * @param chan
         * @param note
         * @param vel
         */
        virtual void sendNoteOn(int chan, int note, int vel) = 0;

        /**
         * @brief sendKeyPressure 0xA
         * @param chan
         * @param note
         * @param value
         */
        virtual void sendKeyPressure(int chan, int note, int value) = 0;

        /**
         * @brief sendController 0xB
         * @param chan
         * @param control
         * @param value
         */
        virtual void sendController(int chan, int control, int value) = 0;

        /**
         * @brief sendProgram 0xC
         * @param chan
         * @param program
         */
        virtual void sendProgram(int chan, int program) = 0;

        /**
         * @brief sendChannelPressure 0xD
         * @param chan
         * @param value
         */
        virtual void sendChannelPressure(int chan, int value) = 0;

        /**
         * @brief sendPitchBend 0xE
         * @param chan
         * @param value
         */
        virtual void sendPitchBend(int chan, int value) = 0;

        /**
         * @brief sendSysex
         * @param data 0xF0 ... 0xF7
         */
        virtual void sendSysex(const QByteArray& data) = 0;

        /**
         * @brief sendSystemMsg
         * @param status 0xF
         */
        virtual void sendSystemMsg(const int status) = 0;
    };
}
}

Q_DECLARE_INTERFACE(drumstick::rt::MIDIOutput, "net.sourceforge.drumstick.rt.MIDIOutput/1.0")

#endif /* MIDIOUTPUT_H */
