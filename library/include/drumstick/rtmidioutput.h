/*
    Drumstick MIDI realtime input-output
    Copyright (C) 2009-2020 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef MIDIOUTPUT_H
#define MIDIOUTPUT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QtPlugin>
#include <QSettings>
#include "macros.h"

/**
 * @file rtmidioutput.h
 * Realtime MIDI output interface
 */

namespace drumstick { namespace rt {

/**
 * @addtogroup RT
 * @{
 */

const quint8 MIDI_STD_CHANNELS              = 16; ///< Standard number of MIDI channels
const quint8 MIDI_GM_STD_DRUM_CHANNEL       = (10-1); ///< Number of the GM percussion channel
const quint8 MIDI_CONTROL_MSB_BANK_SELECT   = 0x00; ///< MIDI Controller number for MSB Bank number
const quint8 MIDI_CONTROL_MSB_MAIN_VOLUME   = 0x07; ///< MIDI Controller number for MSB volume
const quint8 MIDI_CONTROL_LSB_BANK_SELECT   = 0x20; ///< MIDI Controller number for LSB Bank number
const quint8 MIDI_CONTROL_REVERB_SEND       = 0x5b; ///< MIDI Controller number for Reverb send
const quint8 MIDI_CONTROL_ALL_SOUNDS_OFF    = 0x78; ///< MIDI Controller number for All sounds off
const quint8 MIDI_CONTROL_ALL_NOTES_OFF     = 0x7b; ///< MIDI Controller number for All notes off
const quint8 MIDI_CONTROL_RESET_CONTROLLERS = 0x79; ///< MIDI Controller number for Reset all controllers

const quint8 MIDI_STATUS_NOTEOFF         = 0x80; ///< MIDI status byte for NOTE OFF messages
const quint8 MIDI_STATUS_NOTEON          = 0x90; ///< MIDI status byte for NOTE ON messages
const quint8 MIDI_STATUS_KEYPRESURE      = 0xa0; ///< MIDI status byte for KEY pressure messages
const quint8 MIDI_STATUS_CONTROLCHANGE   = 0xb0; ///< MIDI status byte for CONTROL change messages
const quint8 MIDI_STATUS_PROGRAMCHANGE   = 0xc0; ///< MIDI status byte for PROGRAM change messages
const quint8 MIDI_STATUS_CHANNELPRESSURE = 0xd0; ///< MIDI status byte for CHANNEL PRESSURE messages
const quint8 MIDI_STATUS_PITCHBEND       = 0xe0; ///< MIDI status byte for PITCH bend messages
const quint8 MIDI_STATUS_SYSEX           = 0xf0; ///< MIDI status byte for System Exclusive START messages
const quint8 MIDI_STATUS_ENDSYSEX        = 0xf7; ///< MIDI status byte for System Exclusive END messages
const quint8 MIDI_STATUS_REALTIME        = 0xf8; ///< Minimum value for MIDI Realtime messages status

const quint8 MIDI_STATUS_MASK            = 0xf0; ///< Mask to extract the MIDI status byte from a MIDI message
const quint8 MIDI_CHANNEL_MASK           = 0x0f; ///< Mask to extract the MIDI channel byte from a MIDI message

const quint8 MIDI_COMMON_QTRFRAME        = 0xF1; ///< MIDI Quarter frame status message
const quint8 MIDI_COMMON_SONGPP          = 0xF2; ///< MIDI Song Position status message
const quint8 MIDI_COMMON_SONGSELECT      = 0xF3; ///< MIDI Song Select status message
const quint8 MIDI_COMMON_TUNEREQ         = 0xF6; ///< MIDI Tune Request status message

const quint8 MIDI_REALTIME_CLOCK         = 0xF8; ///< MIDI Clock status message
const quint8 MIDI_REALTIME_START         = 0xFA; ///< MIDI Start status message
const quint8 MIDI_REALTIME_CONTINUE      = 0xFB; ///< MIDI Continue status message
const quint8 MIDI_REALTIME_STOP          = 0xFC; ///< MIDI Stop status message
const quint8 MIDI_REALTIME_SENSING       = 0xFE; ///< MIDI Active Sensing status message
const quint8 MIDI_REALTIME_RESET         = 0xFF; ///< MIDI Reset status message

/**
 * @brief MIDI_LSB is a function to extract the least significative byte of a MIDI value
 * @param x a MIDI integer value
 * @return the least significative byte of the input value
 */
inline int MIDI_LSB(int x)
{
    return (x % 0x80);
}

/**
 * @brief MIDI_MSB is a function to extract the most significative byte of a MIDI value
 * @param x MIDI integer value
 * @return the most significative byte of the input value
 */
inline int MIDI_MSB(int x)
{
    return (x / 0x80);
}

    /**
     * @brief MIDIConnection represents a connection identifier
     *
     * MIDIConnection is an alias for QPair<QString,QVariant> where the
     * first component is a QString representing the symbolic name of the MIDI Port
     * and the second component is a QVariant that represents the native identification
     * of the MIDI port, which may be a string, a number, or any other data type
     * accepted as a QVariant.
     */
    typedef QPair<QString,QVariant> MIDIConnection;

    /**
     * @brief MIDI OUT interface
     */
    class DRUMSTICK_EXPORT MIDIOutput : public QObject
    {
        Q_OBJECT

    public:
        /**
         * @brief MIDIOutput constructor
         * @param parent
         */
        explicit MIDIOutput(QObject *parent = nullptr) : QObject(parent) {}
        /**
         * @brief ~MIDIOutput destructor
         */
        virtual ~MIDIOutput() = default;
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
         * @param advanced whether the advanced connections are included or not
         * @return list of available MIDI ports
         */
        virtual QList<MIDIConnection> connections(bool advanced = false) = 0;
        /**
         * @brief setExcludedConnections
         * @param conns
         */
        virtual void setExcludedConnections(QStringList conns) = 0;
        /**
         * @brief open the MIDI port by name
         * @param conn the MIDI connection to be opened
         */
        virtual void open(const MIDIConnection& conn) = 0;
        /**
         * @brief close the MIDI port
         */
        virtual void close() = 0;
        /**
         * @brief currentConnection
         * @return name of the current connection if it is opened
         */
        virtual MIDIConnection currentConnection() = 0;

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

    /** @} */

}} // namespace drumstick::rt

Q_DECLARE_INTERFACE(drumstick::rt::MIDIOutput, "net.sourceforge.drumstick.rt.MIDIOutput/2.0")
Q_DECLARE_METATYPE(drumstick::rt::MIDIConnection);

#endif /* MIDIOUTPUT_H */
