/*
    Standard MIDI File component
    Copyright (C) 2006-2010, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    Based on midifile.c by Tim Thompson, M.Czeiszperger and Greg Lee

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

#ifndef DRUMSTICK_QSMF_H
#define DRUMSTICK_QSMF_H

#include "macros.h"
#include <QObject>

class QDataStream;

/**
 * @file qsmf.h
 * Standard MIDI Files Input/Output
 * @defgroup SMF Standard MIDI Files management (I/O)
 * @{
 */

namespace drumstick {

#define MThd 0x4d546864 /**< SMF Header prefix */
#define MTrk 0x4d54726b /**< SMF Track prefix */

/* Standard MIDI Files meta event definitions */
#define meta_event          0xff /**< SMF Meta Event prefix */
#define sequence_number     0x00 /**< SMF Sequence number */
#define text_event          0x01 /**< SMF Text event */
#define copyright_notice    0x02 /**< SMF Copyright notice */
#define sequence_name       0x03 /**< SMF Sequence name */
#define instrument_name     0x04 /**< SMF Instrument name */
#define lyric               0x05 /**< SMF Lyric */
#define marker              0x06 /**< SMF Marker */
#define cue_point           0x07 /**< SMF Cue point */
#define forced_channel      0x20 /**< SMF Forced MIDI channel */
#define forced_port         0x21 /**< SMF Forced MIDI port */
#define end_of_track        0x2f /**< SMF End of track */
#define set_tempo           0x51 /**< SMF Tempo change */
#define smpte_offset        0x54 /**< SMF SMPTE offset */
#define time_signature      0x58 /**< SMF Time signature */
#define key_signature       0x59 /**< SMF Key signature */
#define sequencer_specific  0x7f /**< SMF Sequencer specific */

/* MIDI status commands most significant bit is 1 */
#define note_off            0x80 /**< MIDI event Note Off */
#define note_on             0x90 /**< MIDI event Note On */
#define poly_aftertouch     0xa0 /**< MIDI event Polyphonic pressure */
#define control_change      0xb0 /**< MIDI event Control change */
#define program_chng        0xc0 /**< MIDI event Program change */
#define channel_aftertouch  0xd0 /**< MIDI event Channel after-touch */
#define pitch_wheel         0xe0 /**< MIDI event Bender */
#define system_exclusive    0xf0 /**< MIDI event System Exclusive begin */
#define end_of_sysex        0xf7 /**< MIDI event System Exclusive end */

#define midi_command_mask   0xf0 /**< Mask to extract the command from the status byte */
#define midi_channel_mask   0x0f /**< Mask to extract the channel from the status byte */

#define major_mode          0 /**< Major mode scale */
#define minor_mode          1 /**< Minor mode scale */

/**
 * Standard MIDI Files input/output
 *
 * This class is used to parse and encode Standard MIDI Files (SMF)
 */
class DRUMSTICK_EXPORT QSmf : public QObject
{
    Q_OBJECT

public:
    QSmf(QObject * parent = 0);
    virtual ~QSmf();

    void readFromStream(QDataStream *stream);
    void readFromFile(const QString& fileName);
    void writeToStream(QDataStream *stream);
    void writeToFile(const QString& fileName);

    void writeMetaEvent(long deltaTime, int type, const QByteArray& data);
    void writeMetaEvent(long deltaTime, int type, const QString& data);
    void writeMetaEvent(long deltaTime, int type, int data);
    void writeMetaEvent(long deltaTime, int type);

    void writeMidiEvent(long deltaTime, int type, int chan, int b1);
    void writeMidiEvent(long deltaTime, int type, int chan, int b1, int b2);
    void writeMidiEvent(long deltaTime, int type, int chan, const QByteArray& data);
    void writeMidiEvent(long deltaTime, int type, long len, char* data);

    void writeTempo(long deltaTime, long tempo);
    void writeBpmTempo(long deltaTime, int tempo);
    void writeTimeSignature(long deltaTime, int num, int den, int cc, int bb);
    void writeKeySignature(long deltaTime, int tone, int mode);
    void writeSequenceNumber(long deltaTime, int seqnum);

    long getCurrentTime();
    long getCurrentTempo();
    long getRealTime();
    long getFilePos();
    int  getDivision();
    void setDivision(int division);
    int  getTracks();
    void setTracks(int tracks);
    int  getFileFormat();
    void setFileFormat(int fileFormat);
    QTextCodec* getTextCodec();
    void setTextCodec(QTextCodec *codec);

signals:
    /**
     * Emitted for a SMF read or write error
     * @param errorStr Error string
     */
    void signalSMFError(const QString& errorStr);
    /**
     * Emitted after reading a SMF header
     * @param format SMF format (0/1)
     * @param ntrks Number pof tracks
     * @param division Division (resolution in ticks per quarter note)
     */
    void signalSMFHeader(int format, int ntrks, int division);
    /**
     * Emitted after reading a Note On message
     * @param chan MIDI Channel
     * @param pitch MIDI Note
     * @param vol Velocity
     */
    void signalSMFNoteOn(int chan, int pitch, int vol);
    /**
     * Emitted after reading a Note Off message
     * @param chan MIDI Channel
     * @param pitch MIDI Note
     * @param vol Velocity
     */
    void signalSMFNoteOff(int chan, int pitch, int vol);
    /**
     * Emitted after reading a Polyphonic Aftertouch message
     * @param chan MIDI Channel
     * @param pitch MIDI Note
     * @param press Pressure amount
     */
    void signalSMFKeyPress(int chan, int pitch, int press);
    /**
     * Emitted after reading a Control Change message
     * @param chan MIDI Channel
     * @param ctl MIDI Controller
     * @param value Control value
     */
    void signalSMFCtlChange(int chan, int ctl, int value);
    /**
     * Emitted after reading a Bender message
     * @param chan MIDI Channel
     * @param value Bender value
     */
    void signalSMFPitchBend(int chan, int value);
    /**
     * Emitted after reading a Program change message
     * @param chan MIDI Channel
     * @param patch Program number
     */
    void signalSMFProgram(int chan, int patch);
    /**
     * Emitted after reading a Channel Aftertouch message
     * @param chan MIDI Channel
     * @param press Pressure amount
     */
    void signalSMFChanPress(int chan, int press);
    /**
     * Emitted after reading a System Exclusive message
     * @param data Sysex bytes
     */
    void signalSMFSysex(const QByteArray& data);
    /**
     * Emitted after reading a Sequencer specific message
     * @param data Message data
     */
    void signalSMFSeqSpecific(const QByteArray& data);
    /**
     * Emitted after reading an unregistered SMF Meta message
     * @param typ Message type
     * @param data Message data
     * @since 0.2.0
     */
    void signalSMFMetaUnregistered(int typ, const QByteArray& data);
    /**
     * Emitted after reading any SMF Meta message
     * @param typ Message type
     * @param data Message data
     */
    void signalSMFMetaMisc(int typ, const QByteArray& data);
    /**
     * Emitted after reading a Sequence number message
     * @param seq Sequence number
     */
    void signalSMFSequenceNum(int seq);
    /**
     * Emitted after reading a Forced channel message
     * @param channel MIDI Channel
     */
    void signalSMFforcedChannel(int channel);
    /**
     * Emitted after reading a Forced port message
     * @param port Port number
     */
    void signalSMFforcedPort(int port);
    /**
     * Emitted after reading a SMF text message
     * @param typ Text type
     * @param data Text data
     */
    void signalSMFText(int typ, const QString& data);
    /**
     * Emitted after reading a SMPT offset message
     * @param b0 Hours
     * @param b1 Minutes
     * @param b2 Seconds
     * @param b3 Frames
     * @param b4 Fractional frames
     */
    void signalSMFSmpte(int b0, int b1, int b2, int b3, int b4);
    /**
     * Emitted after reading a SMF Time signature message
     * @param b0 Numerator
     * @param b1 Denominator (exponent in a power of two)
     * @param b2 Number of MIDI clocks per metronome click
     * @param b3 Number of notated 32nd notes per 24 MIDI clocks
     */
    void signalSMFTimeSig(int b0, int b1, int b2, int b3);
    /**
     * Emitted after reading a SMF Key Signature smessage
     * @param b0 Number of alterations (negative=flats, positive=sharps)
     * @param b1 Scale Mode (0=major, 1=minor)
     */
    void signalSMFKeySig(int b0, int b1);
    /**
     * Emitted after reading a Tempo Change message
     * @param tempo Microseconds per quarter note
     */
    void signalSMFTempo(int tempo);
    /**
     * Emitted after reading a End-Of-Track message
     */
    void signalSMFendOfTrack();
    /**
     * Emitted after reading a track prefix
     */
    void signalSMFTrackStart();
    /**
     * Emitted after a track has finished
     */
    void signalSMFTrackEnd();
    /**
     * Emitted to request the user to write the tempo track.
     * This is the first track in a format 1 SMF.
     */
    void signalSMFWriteTempoTrack();
    /**
     * Emitted to request the user to write a track.
     * @param track Track number
     */
    void signalSMFWriteTrack(int track);

private:
    /**
     * Tempo change within a SMF or sequence
     */
    struct QSmfRecTempo
    {
        quint64 tempo;
        quint64 time;
    };

    class QSmfPrivate;
    QSmfPrivate *d;

    void SMFRead();
    void SMFWrite();
    quint8 getByte();
    void putByte(quint8 value);
    void readHeader();
    void readTrack();
    quint16 to16bit(quint8 c1, quint8 c2);
    quint32 to32bit(quint8 c1, quint8 c2, quint8 c3, quint8 c4);
    quint16 read16bit();
    quint32 read32bit();
    void write16bit(quint16 data);
    void write32bit(quint32 data);
    void writeVarLen(quint64 value);
    double ticksToSecs(quint64 ticks, quint16 division, quint64 tempo);
    long readVarLen();
    void readExpected(const QString& s);
    void addTempo(quint64 tempo, quint64 time);
    quint64 findTempo();
    void SMFError(const QString& s);
    void channelMessage(quint8 status, quint8 c1, quint8 c2);
    void msgInit();
    void msgAdd(quint8 b);
    void metaEvent(quint8 b);
    void sysEx();
    void badByte(quint8 b, int p);
    quint8 lowerByte(quint16 x);
    quint8 upperByte(quint16 x);
    bool endOfSmf();
    void writeHeaderChunk(int format, int ntracks, int division);
    void writeTrackChunk(int track);
};

} /* namespace drumstick */

/** @} */

#endif /* DRUMSTICK_QSMF_H */
