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

#include <QObject>

class QDataStream;

namespace drumstick {

enum WrkChunkType {
    TRACK_CHUNK = 1,
    STREAM_CHUNK = 2,
    VARS_CHUNK = 3,
    TEMPO_CHUNK = 4,
    METER_CHUNK = 5,
    SYSEX_CHUNK = 6,
    MEMRGN_CHUNK = 7,
    COMMENTS_CHUNK = 8,
    TRKOFFS_CHUNK = 9,
    TIMEBASE_CHUNK = 10, // if present should be the first chunk in the file.
    TIMEFMT_CHUNK = 11,
    TRKREPS_CHUNK = 12,
    TRKPATCH_CHUNK = 14,
    THRU_CHUNK = 16,
    LYRICS_CHUNK = 18,
    TRKVOL_CHUNK = 19,
    SYSEX2_CHUNK = 20,
    STRTAB_CHUNK = 22,
    METERKEY_CHUNK = 23,
    TRKNAME_CHUNK = 24,
    VARIABLE_CHUNK = 26,
    NTRKOFS_CHUNK = 27,
    TRKBANK_CHUNK = 30,
    NTRACK_CHUNK = 36,
    NSYSEX_CHUNK = 44,
    NSTREAM_CHUNK = 45,
    SGMNT_CHUNK = 49,
    SOFTVER_CHUNK = 74,
    END_CHUNK = 255
};

const QByteArray HEADER("CAKEWALK");

class QWrk : public QObject
{
    Q_OBJECT

public:
    QWrk(QObject * parent = 0);
    virtual ~QWrk();

    void readFromStream(QDataStream *stream);
    void readFromFile(const QString& fileName);
    QTextCodec* getTextCodec();
    void setTextCodec(QTextCodec *codec);

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
    int getStopTime() const;
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

Q_SIGNALS:

    /**
     * Emitted for a WRK file read error
     * @param errorStr Error string
     */
    void signalWRKError(const QString& errorStr);

    /**
     * Emitted after reading an unknown chunk
     * @param type The chunk type
     * @param length The chunk data
     */
    void signalWRKUnknownChunk(int type, const QByteArray& data);

    /**
     * Emitted after reading a WRK header
     * @param verh WRK file format version major
     * @param verl WRK file format version minor
     */
    void signalWRKHeader(int verh, int verl);

    /**
     * Emitted after reading a Note message
     * @param track track number
     * @param time musical time
     * @param chan MIDI Channel
     * @param pitch MIDI Note
     * @param vol Velocity
     * @param dur Duration
     */
    void signalWRKNote(int track, int time, int chan, int pitch, int vol, int dur);

    /**
     * Emitted after reading a Polyphonic Aftertouch message
     * @param track track number
     * @param time musical time
     * @param chan MIDI Channel
     * @param pitch MIDI Note
     * @param press Pressure amount
     */
    void signalWRKKeyPress(int track, int time, int chan, int pitch, int press);
    /**
     * Emitted after reading a Control Change message
     * @param track track number
     * @param time musical time
     * @param chan MIDI Channel
     * @param ctl MIDI Controller
     * @param value Control value
     */
    void signalWRKCtlChange(int track, int time, int chan, int ctl, int value);
    /**
     * Emitted after reading a Bender message
     * @param track track number
     * @param time musical time
     * @param chan MIDI Channel
     * @param value Bender value
     */
    void signalWRKPitchBend(int track, int time, int chan, int value);
    /**
     * Emitted after reading a Program change message
     * @param track track number
     * @param time musical time
     * @param chan MIDI Channel
     * @param patch Program number
     */
    void signalWRKProgram(int track, int time, int chan, int patch);

    /**
     * Emitted after reading a Channel Aftertouch message
     * @param track track number
     * @param time musical time
     * @param chan MIDI Channel
     * @param press Pressure amount
     */
    void signalWRKChanPress(int track, int time, int chan, int press);

    /**
     * Emitted after reading a System Exclusive event
     * @param track track number
     * @param time musical time
     * @param bank Sysex Bank number
     */
    void signalWRKSysexEvent(int track, int time, int bank);

    /**
     * Emitted after reading a System Exclusive Bank
     * @param bank Sysex Bank number
     * @param name Sysex Bank name
     * @param autosend Send automatically after loading the song
     * @param port MIDI output port
     * @param data Sysex bytes
     */
    void signalWRKSysex(int bank, const QString& name, bool autosend, int port, const QByteArray& data);

    /**
     * Emitted after reading a text message
     * @param track track number
     * @param time musical time
     * @param type Text type
     * @param data Text data
     */
    void signalWRKText(int track, int time, int type, const QString& data);

    /**
     * Emitted after reading a WRK Time signature
     * @param bar Measure number
     * @param num Numerator
     * @param den Denominator (exponent in a power of two)
     */
    void signalWRKTimeSig(int bar, int num, int den);

    /**
     * Emitted after reading a WRK Key Signature
     * @param bar Measure number
     * @param alt Number of alterations (negative=flats, positive=sharps)
     */
    void signalWRKKeySig(int bar, int alt);

    /**
     * Emitted after reading a Tempo Change message
     * @param time musical time
     * @param tempo Microseconds per quarter note
     */
    void signalWRKTempo(int time, int tempo);

    /**
     * Emitted after reading a track prefix
     */
    void signalWRKTrack(const QString& name1,
                        const QString& name2,
                        int trackno, int channel, int pitch,
                        int velocity, int port,
                        bool selected, bool muted, bool loop );
    /**
     * Emitted after reading the timebase chunk
     * @param timebase ticks per quarter note
     */
    void signalWRKTimeBase(int timebase);

    /**
     * Emitted after reading the global variables chunk
     */
    void signalWRKGlobalVars();

    /**
     * Emitted after reading
     */
    void signalWRKThru(int mode, int port, int channel, int keyPlus, int velPlus, int localPort);

    /**
     * Emitted after reading
     */
    void signalWRKTrackOffset(int track, int offset);

    /**
     * Emitted after reading
     */
    void signalWRKTrackReps(int track, int reps);

    /**
     * Emitted after reading
     */
    void signalWRKTrackPatch(int track, int patch);

    /**
     * Emitted after reading
     */
    void signalWRKTrackBank(int track, int bank);

    /**
     * Emitted after reading
     */
    void signalWRKTimeFormat(int frames, int offset);

    /**
     * Emitted after reading
     */
    void signalWRKComments(const QString& data);

    /**
     * Emitted after reading
     */
    void signalWRKVariableRecord(const QString& name, const QByteArray& data);

    /**
     * Emitted after reading
     */
    void signalWRKTrackVol(int track, int vol);

    /**
     * Emitted after reading a new track prefix
     */
    void signalWRKNewTrack(const QString& name,
                           int trackno, int channel, int pitch,
                           int velocity, int port,
                           bool selected, bool muted, bool loop );

    /**
     * Emitted after reading
     */
    void signalWRKSoftVer(const QString& version);

    /**
     * Emitted after reading
     */
    void signalWRKTrackName(int track, const QString& name);

    /**
     * Emitted after reading
     */
    void signalWRKStringTable(const QStringList& strs);

    /**
     * Emitted after reading
     */
    void signalWRKSegment(int track, int time, const QString& name);

    /**
     * Emitted after reading
     */
    void signalWRKChord(int track, int time, const QString& name, const QByteArray& data);

    /**
     * Emitted after reading
     */
    void signalWRKExpression(int track, int time, int code, const QString& text);

    /**
     * Emitted after reading
     */
    void signalWRKHairpin(int track, int time, int code, int dur);

private:
    quint8 readByte();
    quint16 to16bit(quint8 c1, quint8 c2);
    quint32 to32bit(quint8 c1, quint8 c2, quint8 c3, quint8 c4);
    quint16 read16bit();
    quint32 read24bit();
    quint32 read32bit();
    QString readString(int len);
    QString readVarString();
    void readGap(int size);
    bool atEnd();
    qint64 currentPos();
    void seek(qint64 pos);
    int readChunk();
    void processTrackChunk();
    void processVarsChunk();
    void processTimebaseChunk();
    void processNoteArray(int track, int events);
    void processStreamChunk();
    void processMeterChunk();
    void processTempoChunk();
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
    void processUnknown(int id, int max);
    void wrkRead();

    class QWrkPrivate;
    QWrkPrivate *d;
};

} // namespace drumstick;

#endif // DRUMSTICK_QWRK_H
