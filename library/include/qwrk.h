/*
    WRK File component
    Copyright (C) 2010, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef DRUMSTICK_QWRK_H
#define DRUMSTICK_QWRK_H

#include "macros.h"
#include <QObject>

class QDataStream;

/**
 * @file qwrk.h
 * Cakewalk WRK Files Input
 * @defgroup WRK Cakewalk WRK File Parser (Input)
 * @{
 */

namespace drumstick {

/**
 * Record types within a WRK file
 */
enum WrkChunkType {
    TRACK_CHUNK = 1,     ///< Track prefix
    STREAM_CHUNK = 2,    ///< Events stream
    VARS_CHUNK = 3,      ///< Global variables
    TEMPO_CHUNK = 4,     ///< Tempo map
    METER_CHUNK = 5,     ///< Meter map
    SYSEX_CHUNK = 6,     ///< System exclusive bank
    MEMRGN_CHUNK = 7,    ///< Memory region
    COMMENTS_CHUNK = 8,  ///< Comments
    TRKOFFS_CHUNK = 9,   ///< Track offset
    TIMEBASE_CHUNK = 10, ///< Timebase. If present is the first chunk in the file.
    TIMEFMT_CHUNK = 11,  ///< SMPTE time format
    TRKREPS_CHUNK = 12,  ///< Track repetitions
    TRKPATCH_CHUNK = 14, ///< Track patch
    NTEMPO_CHUNK = 15,   ///< New Tempo map
    THRU_CHUNK = 16,     ///< Extended thru parameters
    LYRICS_CHUNK = 18,   ///< Events stream with lyrics
    TRKVOL_CHUNK = 19,   ///< Track volume
    SYSEX2_CHUNK = 20,   ///< System exclusive bank
    STRTAB_CHUNK = 22,   ///< Table of text event types
    METERKEY_CHUNK = 23, ///< Meter/Key map
    TRKNAME_CHUNK = 24,  ///< Track name
    VARIABLE_CHUNK = 26, ///< Variable record chunk
    NTRKOFS_CHUNK = 27,  ///< Track offset
    TRKBANK_CHUNK = 30,  ///< Track bank
    NTRACK_CHUNK = 36,   ///< Track prefix
    NSYSEX_CHUNK = 44,   ///< System exclusive bank
    NSTREAM_CHUNK = 45,  ///< Events stream
    SGMNT_CHUNK = 49,    ///< Segment prefix
    SOFTVER_CHUNK = 74,  ///< Software version which saved the file
    END_CHUNK = 255      ///< Last chunk, end of file
};

const QByteArray HEADER("CAKEWALK"); ///< Cakewalk WRK File header id

/**
 * Cakewalk WRK file format (input only)
 *
 * This class is used to parse Cakewalk WRK Files
 */
class DRUMSTICK_EXPORT QWrk : public QObject
{
    Q_OBJECT

public:
    QWrk(QObject * parent = 0);
    virtual ~QWrk();

    void readFromStream(QDataStream *stream);
    void readFromFile(const QString& fileName);
    QTextCodec* getTextCodec();
    void setTextCodec(QTextCodec *codec);
    long getFilePos();

    int getNow() const;
    int getFrom() const;
    int getThru() const;
    int getKeySig() const;
    int getClock() const;
    int getAutoSave() const;
    int getPlayDelay() const;
    bool getZeroCtrls() const;
    bool getSendSPP() const;
    bool getSendCont() const;
    bool getPatchSearch() const;
    bool getAutoStop() const;
    unsigned int getStopTime() const;
    bool getAutoRewind() const;
    int getRewindTime() const;
    bool getMetroPlay() const;
    bool getMetroRecord() const;
    bool getMetroAccent() const;
    int getCountIn() const;
    bool getThruOn() const;
    bool getAutoRestart() const;
    int getCurTempoOfs() const;
    int getTempoOfs1() const;
    int getTempoOfs2() const;
    int getTempoOfs3() const;
    bool getPunchEnabled() const;
    int getPunchInTime() const;
    int getPunchOutTime() const;
    int getEndAllTime() const;

    QByteArray getLastChunkRawData() const;
    double getRealTime(long ticks) const;

Q_SIGNALS:

    /**
     * Emitted for a WRK file read error
     *
     * @param errorStr Error string
     */
    void signalWRKError(const QString& errorStr);

    /**
     * Emitted after reading an unknown chunk
     *
     * @param type chunk type
     * @param data chunk data (not decoded)
     */
    void signalWRKUnknownChunk(int type, const QByteArray& data);

    /**
     * Emitted after reading a WRK header
     *
     * @param verh WRK file format version major
     * @param verl WRK file format version minor
     */
    void signalWRKHeader(int verh, int verl);

    /**
     * Emitted after reading the last chunk of a WRK file
     */
    void signalWRKEnd();

    /**
     * Emitted after reading the last event of a event stream
     */
    void signalWRKStreamEnd(long time);

    /**
     * Emitted after reading a Note message
     *
     * @param track track number
     * @param time musical time
     * @param chan MIDI Channel
     * @param pitch MIDI Note
     * @param vol Velocity
     * @param dur Duration
     */
    void signalWRKNote(int track, long time, int chan, int pitch, int vol, int dur);

    /**
     * Emitted after reading a Polyphonic Aftertouch message
     *
     * @param track track number
     * @param time musical time
     * @param chan MIDI Channel
     * @param pitch MIDI Note
     * @param press Pressure amount
     */
    void signalWRKKeyPress(int track, long time, int chan, int pitch, int press);

    /**
     * Emitted after reading a Control Change message
     *
     * @param track track number
     * @param time musical time
     * @param chan MIDI Channel
     * @param ctl MIDI Controller
     * @param value Control value
     */
    void signalWRKCtlChange(int track, long time, int chan, int ctl, int value);

    /**
     * Emitted after reading a Bender message
     *
     * @param track track number
     * @param time musical time
     * @param chan MIDI Channel
     * @param value Bender value
     */
    void signalWRKPitchBend(int track, long time, int chan, int value);

    /**
     * Emitted after reading a Program change message
     *
     * @param track track number
     * @param time musical time
     * @param chan MIDI Channel
     * @param patch Program number
     */
    void signalWRKProgram(int track, long time, int chan, int patch);

    /**
     * Emitted after reading a Channel Aftertouch message
     *
     * @param track track number
     * @param time musical time
     * @param chan MIDI Channel
     * @param press Pressure amount
     */
    void signalWRKChanPress(int track, long time, int chan, int press);

    /**
     * Emitted after reading a System Exclusive event
     *
     * @param track track number
     * @param time musical time
     * @param bank Sysex Bank number
     */
    void signalWRKSysexEvent(int track, long time, int bank);

    /**
     * Emitted after reading a System Exclusive Bank
     *
     * @param bank Sysex Bank number
     * @param name Sysex Bank name
     * @param autosend Send automatically after loading the song
     * @param port MIDI output port
     * @param data Sysex bytes
     */
    void signalWRKSysex(int bank, const QString& name, bool autosend, int port, const QByteArray& data);

    /**
     * Emitted after reading a text message
     *
     * @param track track number
     * @param time musical time
     * @param type Text type
     * @param data Text data
     */
    void signalWRKText(int track, long time, int type, const QString& data);

    /**
     * Emitted after reading a WRK Time signature
     *
     * @param bar Measure number
     * @param num Numerator
     * @param den Denominator (exponent in a power of two)
     */
    void signalWRKTimeSig(int bar, int num, int den);

    /**
     * Emitted after reading a WRK Key Signature
     *
     * @param bar Measure number
     * @param alt Number of alterations (negative=flats, positive=sharps)
     */
    void signalWRKKeySig(int bar, int alt);

    /**
     * Emitted after reading a Tempo Change message.
     *
     * Tempo units are given in beats * 100 per minute, so to obtain BPM
     * it is necessary to divide by 100 the tempo.
     *
     * @param time musical time
     * @param tempo beats per minute multiplied by 100
     */
    void signalWRKTempo(long time, int tempo);

    /**
     * Emitted after reading a track prefix chunk
     *
     * @param name1 track 1st name
     * @param name2 track 2nd name
     * @param trackno track number
     * @param channel track forced channel (-1=no forced)
     * @param pitch track pitch transpose in semitones (-127..127)
     * @param velocity track velocity increment (-127..127)
     * @param port track forced port
     * @param selected true if track is selected
     * @param muted true if track is muted
     * @param loop true if loop is enabled
     */
    void signalWRKTrack(const QString& name1,
                        const QString& name2,
                        int trackno, int channel, int pitch,
                        int velocity, int port,
                        bool selected, bool muted, bool loop );

    /**
     * Emitted after reading the timebase chunk
     *
     * @param timebase ticks per quarter note
     */
    void signalWRKTimeBase(int timebase);

    /**
     * Emitted after reading the global variables chunk.
     *
     * This record contains miscellaneous Cakewalk global variables that can
     * be retrieved using individual getters.
     *
     * @see getNow(), getFrom(), getThru()
     */
    void signalWRKGlobalVars();

    /**
     * Emitted after reading an Extended Thru parameters chunk.
     *
     * It was introduced in Cakewalk version 4.0.  These parameters are
     * intended to override the global vars Thruon value, so this record should
     * come after the VARS_CHUNK record. It is optional.
     *
     * @param mode (auto, off, on)
     * @param port MIDI port
     * @param channel MIDI channel
     * @param keyPlus Note transpose
     * @param velPlus Velocity transpose
     * @param localPort MIDI local port
     */
    void signalWRKThru(int mode, int port, int channel, int keyPlus, int velPlus, int localPort);

    /**
     * Emitted after reading a track offset chunk
     *
     * @param track track number
     * @param offset time offset
     */
    void signalWRKTrackOffset(int track, int offset);

    /**
     * Emitted after reading a track offset chunk
     *
     * @param track track number
     * @param reps number of repetitions
     */
    void signalWRKTrackReps(int track, int reps);

    /**
     * Emitted after reading a track patch chunk
     *
     * @param track track number
     * @param patch
     */
    void signalWRKTrackPatch(int track, int patch);

    /**
     * Emitted after reading a track bank chunk
     *
     * @param track track number
     * @param bank
     */
    void signalWRKTrackBank(int track, int bank);

    /**
     * Emitted after reading a SMPTE time format chunk
     *
     * @param frames frames/sec (24, 25, 29=30-drop, 30)
     * @param offset frames of offset
     */
    void signalWRKTimeFormat(int frames, int offset);

    /**
     * Emitted after reading a comments chunk
     *
     * @param data file text comments
     */
    void signalWRKComments(const QString& data);

    /**
     * Emitted after reading a variable chunk.
     * This record may contain data in text or binary format.
     *
     * @param name record identifier
     * @param data record variable data
     */
    void signalWRKVariableRecord(const QString& name, const QByteArray& data);

    /**
     * Emitted after reading a track volume chunk.
     *
     * @param track track number
     * @param vol initial volume
     */
    void signalWRKTrackVol(int track, int vol);

    /**
     * Emitted after reading a new track prefix
     *
     * @param name track name
     * @param trackno track number
     * @param channel forced MIDI channel
     * @param pitch Note transposition
     * @param velocity Velocity increment
     * @param port MIDI port number
     * @param selected track is selected
     * @param muted track is muted
     * @param loop track loop enabled
     */
    void signalWRKNewTrack( const QString& name,
                            int trackno, int channel, int pitch,
                            int velocity, int port,
                            bool selected, bool muted, bool loop );

    /**
     * Emitted after reading a software version chunk.
     *
     * @param version software version string
     */
    void signalWRKSoftVer(const QString& version);

    /**
     * Emitted after reading a track name chunk.
     *
     * @param track track number
     * @param name track name
     */
    void signalWRKTrackName(int track, const QString& name);

    /**
     * Emitted after reading a string event types chunk.
     *
     * @param strs list of declared string event types
     */
    void signalWRKStringTable(const QStringList& strs);

    /**
     * Emitted after reading a segment prefix chunk.
     *
     * @param track track number
     * @param time segment time offset
     * @param name segment name
     */
    void signalWRKSegment(int track, long time, const QString& name);

    /**
     * Emitted after reading a chord diagram chunk.
     *
     * @param track track number
     * @param time event time in ticks
     * @param name chord name
     * @param data chord data definition (not decoded)
     */
    void signalWRKChord(int track, long time, const QString& name, const QByteArray& data);

    /**
     * Emitted after reading an expression indication (notation) chunk.
     *
     * @param track track number
     * @param time event time in ticks
     * @param code expression event code
     * @param text expression text
     */
    void signalWRKExpression(int track, long time, int code, const QString& text);

    /**
     * Emitted after reading a hairpin symbol (notation) chunk.
     *
     * @param track track number
     * @param time event time in ticks
     * @param code hairpin code
     * @param dur duration
     */
    void signalWRKHairpin(int track, long time, int code, int dur);

private:
    quint8 readByte();
    quint16 to16bit(quint8 c1, quint8 c2);
    quint32 to32bit(quint8 c1, quint8 c2, quint8 c3, quint8 c4);
    quint16 read16bit();
    quint32 read24bit();
    quint32 read32bit();
    QString readString(int len);
    QString readVarString();
    void readRawData(int size);
    void readGap(int size);
    bool atEnd();
    void seek(qint64 pos);

    int readChunk();
    void processTrackChunk();
    void processVarsChunk();
    void processTimebaseChunk();
    void processNoteArray(int track, int events);
    void processStreamChunk();
    void processMeterChunk();
    void processTempoChunk(int factor = 1);
    void processSysexChunk();
    void processSysex2Chunk();
    void processNewSysexChunk();
    void processThruChunk();
    void processTrackOffset();
    void processTrackReps();
    void processTrackPatch();
    void processTrackBank();
    void processTimeFormat();
    void processComments();
    void processVariableRecord(int max);
    void processNewTrack();
    void processSoftVer();
    void processTrackName();
    void processStringTable();
    void processLyricsStream();
    void processTrackVol();
    void processNewTrackOffset();
    void processMeterKeyChunk();
    void processSegmentChunk();
    void processNewStream();
    void processUnknown(int id);
    void processEndChunk();
    void wrkRead();

    struct RecTempo {
        long time;
        double tempo;
        double seconds;
    };

    class QWrkPrivate;
    QWrkPrivate *d;
};

} // namespace drumstick;

/** @} */

#endif // DRUMSTICK_QWRK_H
