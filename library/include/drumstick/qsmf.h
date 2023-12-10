/*
    Standard MIDI File component
    Copyright (C) 2006-2023, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    Based on midifile.c by Tim Thompson, M.Czeiszperger and Greg Lee

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

#ifndef DRUMSTICK_QSMF_H
#define DRUMSTICK_QSMF_H

#include "macros.h"
#include <QObject>
#include <QScopedPointer>

class QTextCodec;
class QDataStream;

/**
 * @file qsmf.h
 * Standard MIDI Files Input/Output
 */

#if defined(DRUMSTICK_STATIC)
#define DRUMSTICK_FILE_EXPORT
#else
#if defined(drumstick_file_EXPORTS)
#define DRUMSTICK_FILE_EXPORT Q_DECL_EXPORT
#else
#define DRUMSTICK_FILE_EXPORT Q_DECL_IMPORT
#endif
#endif

namespace drumstick {
/**
 * @ingroup File
 * @brief Drumstick File library
 */
namespace File {

/**
 * @addtogroup SMF Standard MIDI Files Management (I/O)
 * @{
 */

const quint32 MThd = 0x4d546864; /**< SMF Header prefix */
const quint32 MTrk = 0x4d54726b; /**< SMF Track prefix */

/* Standard MIDI Files meta event definitions */
const quint8 meta_event =         0xff; /**< SMF Meta Event prefix */
const quint8 sequence_number =    0x00; /**< SMF Sequence number */
const quint8 text_event =         0x01; /**< SMF Text event */
const quint8 copyright_notice =   0x02; /**< SMF Copyright notice */
const quint8 sequence_name =      0x03; /**< SMF Sequence name */
const quint8 instrument_name =    0x04; /**< SMF Instrument name */
const quint8 lyric =              0x05; /**< SMF Lyric */
const quint8 marker =             0x06; /**< SMF Marker */
const quint8 cue_point =          0x07; /**< SMF Cue point */
const quint8 forced_channel =     0x20; /**< SMF Forced MIDI channel */
const quint8 forced_port =        0x21; /**< SMF Forced MIDI port */
const quint8 end_of_track =       0x2f; /**< SMF End of track */
const quint8 set_tempo =          0x51; /**< SMF Tempo change */
const quint8 smpte_offset =       0x54; /**< SMF SMPTE offset */
const quint8 time_signature =     0x58; /**< SMF Time signature */
const quint8 key_signature =      0x59; /**< SMF Key signature */
const quint8 sequencer_specific = 0x7f; /**< SMF Sequencer specific */

/* MIDI status commands most significant bit is 1 */
const quint8 note_off =           0x80; /**< MIDI event Note Off */
const quint8 note_on =            0x90; /**< MIDI event Note On */
const quint8 poly_aftertouch =    0xa0; /**< MIDI event Polyphonic pressure */
const quint8 control_change =     0xb0; /**< MIDI event Control change */
const quint8 program_chng =       0xc0; /**< MIDI event Program change */
const quint8 channel_aftertouch = 0xd0; /**< MIDI event Channel after-touch */
const quint8 pitch_wheel =        0xe0; /**< MIDI event Bender */
const quint8 system_exclusive =   0xf0; /**< MIDI event System Exclusive begin */
const quint8 end_of_sysex =       0xf7; /**< MIDI event System Exclusive end */

const quint8 midi_command_mask =  0xf0; /**< Mask to extract the command from the status byte */
const quint8 midi_channel_mask =  0x0f; /**< Mask to extract the channel from the status byte */

const quint8 major_mode =         0; /**< Major mode scale */
const quint8 minor_mode =         1; /**< Minor mode scale */

/**
 * Standard MIDI Files input/output
 *
 * This class is used to parse and encode Standard MIDI Files (SMF)
 */
class DRUMSTICK_FILE_EXPORT QSmf : public QObject
{
    Q_OBJECT

public:
    explicit QSmf(QObject * parent = nullptr);
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
    Q_DECL_DEPRECATED QTextCodec* getTextCodec();
    Q_DECL_DEPRECATED void setTextCodec(QTextCodec *codec);

Q_SIGNALS:
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
     * @deprecated because the class QTextCodec was removed from QtCore since Qt6.
     * use signalSMFText2() instead.
     */
    Q_DECL_DEPRECATED void signalSMFText(int typ, const QString& data);
    /**
     * Emitted after reading a SMF text message
     * @param typ Text type
     * @param data Text data
     */
    void signalSMFText2(int typ, const QByteArray& data);
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
     * Emitted to request the user to prepare the tempo track.
     *
     * The signal handler should not call the writeTempo() or writeBpmTempo()
     * methods directly, but instead it should fill the Conductor track with
     * tempo and other SMF meta events like key and time signatures to be written
     * later, at the signalSMFWriteTrack() handler.
     *
     * The Conductor track is the first track in a format 1 SMF.
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
    QScopedPointer<QSmfPrivate> d;

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

QString DRUMSTICK_FILE_EXPORT drumstickLibraryVersion();

/** @} */

}} /* namespace drumstick::File */

#endif /* DRUMSTICK_QSMF_H */
