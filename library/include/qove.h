/*
    Overture OVE File component
    Copyright (C) 2006-2015, Rui Fan <vanferry@gmail.com>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef DRUMSTICK_OVE_H
#define DRUMSTICK_OVE_H

#include "macros.h"
#include <QObject>

namespace OVE {
    class Track;
    class Voice;
    class Measure;
    class MeasureData;
    class NoteContainer;
}

/**
 * @file qove.h
 * Overture OVE Files Input
 * @defgroup OVE Overture File Parser (Input)
 * @{
 */

namespace drumstick {

class MeasureToTick;

/**
 * Overture OVE Files (input only)
 *
 * This class is used to parse Overture Files
 * @since 0.5.0
 */
class DRUMSTICK_EXPORT QOve : public QObject {
    Q_OBJECT

public:
    QOve(QObject * parent = 0);
    virtual ~QOve();

    void setTextCodecName(const QString& codec);

    void readFromFile(const QString& fileName);

Q_SIGNALS:

	/**
	 * Emitted for a OVE file read error
	 *
	 * @param errorStr Error string
	 */
	void signalOVEError(const QString& errorStr);

	/**
	 * Emitted after reading a WRK header
	 *
	 * @param quarter of OVE file
	 * @param trackCount of OVE file
	 */
	void signalOVEHeader(int quarter, int trackCount);

	/**
	 * Emitted after reading the last chunk of a OVE file
	 */
	void signalOVEEnd();

	/**
	 * Emitted after reading a Note message
	 *
	 * @param track track number
	 * @param tick musical time
	 * @param channel MIDI Channel
	 * @param pitch MIDI Note
	 * @param vol Velocity
	 */
	void signalOVENoteOn(int track, long tick, int channel, int pitch, int vol);
	void signalOVENoteOff(int track, long tick, int channel, int pitch, int vol);

	/**
	 * Emitted after reading a Control Change message
	 *
	 * @param track track number
	 * @param tick musical time
	 * @param channel MIDI Channel
	 * @param ctl MIDI Controller
	 * @param value Control value
	 */
	void signalOVECtlChange(int track, long tick, int channel, int ctl, int value);

	/**
	 * Emitted after reading a Polyphonic Aftertouch message (Key Aftertouch)
	 *
	 * @param track track number
	 * @param tick musical time
	 * @param channel MIDI Channel
	 * @param pitch MIDI Note
	 * @param press Pressure amount
	 */
	void signalOVEKeyPress(int track, long tick, int channel, int pitch, int press);

	/**
	 * Emitted after reading a Bender message
	 *
	 * @param track track number
	 * @param tick musical time
	 * @param channel MIDI Channel
	 * @param value Bender value
	 */
	void signalOVEPitchBend(int track, long tick, int channel, int value);

	/**
	 * Emitted after reading a Program change message
	 *
	 * @param track track number
	 * @param tick musical time
	 * @param channel MIDI Channel
	 * @param patch Program number
	 */
	void signalOVEProgram(int track, long tick, int channel, int patch);

	/**
	 * Emitted after reading a Channel Aftertouch message
	 *
	 * @param track track number
	 * @param tick musical time
	 * @param channel MIDI Channel
	 * @param press Pressure amount
	 */
	void signalOVEChanPress(int track, long tick, int channel, int press);

	/**
	 * Emitted after reading a System Exclusive event
	 *
	 * @param track track number
	 * @param tick musical time
	 * @param bank Sysex Bank number
	 */
	void signalOVESysexEvent(int track, long tick, int bank);

	/**
	 * Emitted after reading a System Exclusive Bank
	 *
	 * @param bank Sysex Bank number
	 * @param name Sysex Bank name
	 * @param autosend Send automatically after loading the song
	 * @param port MIDI output port
	 * @param data Sysex bytes
	 */
	void signalOVESysex(int bank, const QString& name, bool autosend, int port, const QByteArray& data);

	/**
	 * Emitted after reading a text message
	 *
	 * @param track track number
	 * @param tick musical time
	 * @param data Text data
	 */
	void signalOVEText(int track, long tick, const QString& data);

	/**
	 * Emitted after reading a Time signature
	 *
	 * @param bar Measure number
     * @param tick musical time
	 * @param num Numerator
	 * @param den Denominator (exponent in a power of two)
	 */
	void signalOVETimeSig(int bar, long tick, int num, int den);

	/**
	 * Emitted after reading a Key Signature
	 *
	 * @param bar Measure number
	 * @param tick musical time
	 * @param alt Number of alterations (negative=flats, positive=sharps)
	 */
	void signalOVEKeySig(int bar, long tick, int alt);

	/**
	 * Emitted after reading a Tempo Change message.
	 *
	 * Tempo units are given in beats * 100 per minute, so to obtain BPM
	 * it is necessary to divide by 100 the tempo.
	 *
	 * @param tick musical time
	 * @param tempo beats per minute multiplied by 100
	 */
	void signalOVETempo(long tick, int tempo);

	/**
	 * Emitted after reading a new track prefix
	 *
	 * @param name track name
	 * @param track track number
	 * @param channel forced MIDI channel
	 * @param pitch Note transposition
	 * @param velocity Velocity increment
	 * @param port MIDI port number
	 * @param selected track is selected
	 * @param muted track is muted
	 * @param loop track loop enabled
	 */
	void signalOVENewTrack(const QString& name, int track, int channel,
			int pitch, int velocity, int port, bool selected, bool muted, bool loop);

	/**
	 * Emitted after reading a track patch chunk
	 *
	 * @param track track number
	 * @param channel MIDI Channel
	 * @param patch
	 */
	void signalOVETrackPatch(int track, int channel, int patch);

	/**
	 * Emitted after reading a track bank chunk
	 *
	 * @param track track number
	 * @param channel MIDI Channel
	 * @param bank
	 */
	void signalOVETrackBank(int track, int channel, int bank);

	/**
	 * Emitted after reading a track volume chunk.
	 *
	 * @param track track number
	 * @param channel MIDI Channel
	 * @param vol initial volume
	 */
	void signalOVETrackVol(int track, int channel, int vol);

	/**
	 * Emitted after reading a chord diagram chunk.
	 *
	 * @param track track number
	 * @param tick event time in ticks
	 * @param name chord name
	 * @param data chord data definition (not decoded)
	 */
	void signalOVEChord(int track, long tick, const QString& name, const QByteArray& data);

	/**
	 * Emitted after reading an expression indication (notation) chunk.
	 *
	 * @param track track number
	 * @param time event time in ticks
	 * @param code expression event code
	 * @param text expression text
	 */
	void signalOVEExpression(int track, long time, int code, const QString& text);

private:
	void convertSong();
	void convertSignatures();
	void convertTrackHeader(OVE::Track* track, int trackNo);
	void convertMeasure(
			OVE::Track* track,
			int trackNo,
			const QList<OVE::Voice*>& voices,
			OVE::Measure* measure,
			OVE::MeasureData* measureData,
			int transpose,
			int offsetTick);
	void convertNotes(int trackNo, int measureTick, OVE::NoteContainer* container, int channel, int pitchShift);

private:
	class QOvePrivate;
	QOvePrivate* d;
};

} // namespace drumstick

/** @} */

#endif // DRUMSTICK_OVE_H
