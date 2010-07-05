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

#include "qwrk.h"
#include <cmath>
#include <QDataStream>
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QTextCodec>
#include <QStringList>

/**
 * @file qwrk.cpp
 * Implementation of a class managing Cakewalk WRK Files input
 */

namespace drumstick {

/**
 * @addtogroup WRK
 * @{
 *
 * QWrk provides a mechanism to parse Cakewalk WRK Files, without
 * the burden of a policy forcing to use some internal sequence representation.
 *
 * This class is not related or based on the ALSA library.
 *
 * @}
 */

class QWrk::QWrkPrivate {
public:
    QWrkPrivate():
    m_Now(0),
    m_From(0),
    m_Thru(11930),
    m_KeySig(0),
    m_Clock(0),
    m_AutoSave(0),
    m_PlayDelay(0),
    m_ZeroCtrls(false),
    m_SendSPP(true),
    m_SendCont(true),
    m_PatchSearch(false),
    m_AutoStop(false),
    m_StopTime(4294967295U),
    m_AutoRewind(false),
    m_RewindTime(0),
    m_MetroPlay(false),
    m_MetroRecord(true),
    m_MetroAccent(false),
    m_CountIn(1),
    m_ThruOn(true),
    m_AutoRestart(false),
    m_CurTempoOfs(1),
    m_TempoOfs1(32),
    m_TempoOfs2(64),
    m_TempoOfs3(128),
    m_PunchEnabled(false),
    m_PunchInTime(0),
    m_PunchOutTime(0),
    m_EndAllTime(0),
    m_division(120),
    m_codec(0),
    m_IOStream(0)
    { }

    quint32 m_Now;          ///< Now marker time
    quint32 m_From;         ///< From marker time
    quint32 m_Thru;         ///< Thru marker time
    quint8 m_KeySig;        ///< Key signature (0=C, 1=C#, ... 11=B)
    quint8 m_Clock;         ///< Clock Source (0=Int, 1=MIDI, 2=FSK, 3=SMPTE)
    quint8 m_AutoSave;      ///< Auto save (0=disabled, 1..256=minutes)
    quint8 m_PlayDelay;     ///< Play Delay
    bool m_ZeroCtrls;       ///< Zero continuous controllers?
    bool m_SendSPP;         ///< Send Song Position Pointer?
    bool m_SendCont;        ///< Send MIDI Continue?
    bool m_PatchSearch;     ///< Patch/controller search-back?
    bool m_AutoStop;        ///< Auto-stop?
    quint32 m_StopTime;     ///< Auto-stop time
    bool m_AutoRewind;      ///< Auto-rewind?
    quint32 m_RewindTime;   ///< Auto-rewind time
    bool m_MetroPlay;       ///< Metronome on during playback?
    bool m_MetroRecord;     ///< Metronome on during recording?
    bool m_MetroAccent;     ///< Metronome accents primary beats?
    quint8 m_CountIn;       ///< Measures of count-in (0=no count-in)
    bool m_ThruOn;          ///< MIDI Thru enabled? (only used if no THRU rec)
    bool m_AutoRestart;     ///< Auto-restart?
    quint8 m_CurTempoOfs;   ///< Which of the 3 tempo offsets is used: 0..2
    quint8 m_TempoOfs1;     ///< Fixed-point ratio value of offset 1
    quint8 m_TempoOfs2;     ///< Fixed-point ratio value of offset 2
    quint8 m_TempoOfs3;     ///< Fixed-point ratio value of offset 3
    bool m_PunchEnabled;    ///< Auto-Punch enabled?
    quint32 m_PunchInTime;  ///< Punch-in time
    quint32 m_PunchOutTime;	///< Punch-out time
    quint32 m_EndAllTime;   ///< Time of latest event (incl. all tracks)

    int m_division;
    QTextCodec *m_codec;
    QDataStream *m_IOStream;
    QByteArray m_lastChunkData;
    QList<RecTempo> m_tempos;
};

QWrk::QWrk(QObject * parent) :
    QObject(parent),
    d(new QWrkPrivate)
{ }

QWrk::~QWrk()
{
    delete d;
}

/**
 * Gets the text codec used for text meta-events I/O
 * @return QTextCodec pointer
 */
QTextCodec* QWrk::getTextCodec()
{
    return d->m_codec;
}

/**
 * Sets the text codec for text meta-events.
 * The engine doesn't take ownership of the codec instance.
 *
 * @param codec QTextCodec pointer
 */
void QWrk::setTextCodec(QTextCodec *codec)
{
    d->m_codec = codec;
}

/**
 * Gets the last chunk raw data (undecoded)
 *
 * @return last chunk raw data
 */
QByteArray QWrk::getLastChunkRawData() const
{
    return d->m_lastChunkData;
}

/**
 * Read the chunk raw data (undecoded)
 */
void QWrk::readRawData(int size)
{
    d->m_lastChunkData = d->m_IOStream->device()->read(size);
}

/**
 * Now marker time
 * @return Now marker time
 */
int QWrk::getNow() const
{
    return d->m_Now;
}

/**
 * From marker time
 * @return From marker time
 */
int QWrk::getFrom() const
{
    return d->m_From;
}

/**
 * Thru marker time
 * @return Thru marker time
 */
int QWrk::getThru() const
{
    return d->m_Thru;
}

/**
 * Key signature (0=C, 1=C#, ... 11=B)
 * @return Key signature
 */
int QWrk::getKeySig() const
{
    return d->m_KeySig;
}

/**
 * Clock Source (0=Int, 1=MIDI, 2=FSK, 3=SMPTE)
 * @return Clock Source
 */
int QWrk::getClock() const
{
    return d->m_Clock;
}

/**
 * Auto save (0=disabled, 1..256=minutes)
 * @return Auto save
 */
int QWrk::getAutoSave() const
{
    return d->m_AutoSave;
}

/**
 * Play Delay
 * @return Play Delay
 */
int QWrk::getPlayDelay() const
{
    return d->m_PlayDelay;
}

/**
 * Zero continuous controllers?
 * @return Zero continuous controllers
 */
bool QWrk::getZeroCtrls() const
{
    return d->m_ZeroCtrls;
}

/**
 * Send Song Position Pointer?
 * @return Send Song Position Pointer
 */
bool QWrk::getSendSPP() const
{
    return d->m_SendSPP;
}

/**
 * Send MIDI Continue?
 * @return Send MIDI Continue
 */
bool QWrk::getSendCont() const
{
    return d->m_SendCont;
}

/**
 * Patch/controller search-back?
 * @return Patch/controller search-back
 */
bool QWrk::getPatchSearch() const
{
    return d->m_PatchSearch;
}

/**
 * Auto-stop?
 * @return Auto-stop
 */
bool QWrk::getAutoStop() const
{
    return d->m_AutoStop;
}

/**
 * Auto-stop time
 * @return Auto-stop time
 */
unsigned int QWrk::getStopTime() const
{
    return d->m_StopTime;
}

/**
 * Auto-rewind?
 * @return Auto-rewind
 */
bool QWrk::getAutoRewind() const
{
    return d->m_AutoRewind;
}

/**
 * Auto-rewind time
 * @return Auto-rewind time
 */
int QWrk::getRewindTime() const
{
    return d->m_RewindTime;
}

/**
 * Metronome on during playback?
 * @return Metronome on during playback
 */
bool QWrk::getMetroPlay() const
{
    return d->m_MetroPlay;
}

/**
 * Metronome on during recording?
 * @return Metronome on during recording
 */
bool QWrk::getMetroRecord() const
{
    return d->m_MetroRecord;
}

/**
 * Metronome accents primary beats?
 * @return Metronome accents primary beats
 */
bool QWrk::getMetroAccent() const
{
    return d->m_MetroAccent;
}

/**
 * Measures of count-in (0=no count-in)
 * @return Measures of count-in
 */
int QWrk::getCountIn() const
{
    return d->m_CountIn;
}

/**
 * MIDI Thru enabled? (only used if no THRU rec)
 * @return MIDI Thru enabled
 */
bool QWrk::getThruOn() const
{
    return d->m_ThruOn;
}

/**
 * Auto-restart?
 * @return Auto-restart
 */
bool QWrk::getAutoRestart() const
{
    return d->m_AutoRestart;
}

/**
 * Which of the 3 tempo offsets is used: 0..2
 * @return tempo offset index
 */
int QWrk::getCurTempoOfs() const
{
    return d->m_CurTempoOfs;
}

/**
 * Fixed-point ratio value of tempo offset 1
 *
 * NOTE: The offset ratios are expressed as a numerator in the expression
 * n/64.  To get a ratio from this number, divide the number by 64.  To get
 * this number from a ratio, multiply the ratio by 64.
 * Examples:
 *   32 ==>  32/64 = 0.5
 *   63 ==>  63/64 = 0.9
 *   64 ==>  64/64 = 1.0
 *  128 ==> 128/64 = 2.0
 *
 * @return tempo offset 1
 */
int QWrk::getTempoOfs1() const
{
    return d->m_TempoOfs1;
}

/**
 * Fixed-point ratio value of tempo offset 2
 *
 * NOTE: The offset ratios are expressed as a numerator in the expression
 * n/64.  To get a ratio from this number, divide the number by 64.  To get
 * this number from a ratio, multiply the ratio by 64.
 * Examples:
 *   32 ==>  32/64 = 0.5
 *   63 ==>  63/64 = 0.9
 *   64 ==>  64/64 = 1.0
 *  128 ==> 128/64 = 2.0
 *
 * @return tempo offset 2
 */
int QWrk::getTempoOfs2() const
{
    return d->m_TempoOfs2;
}

/**
 * Fixed-point ratio value of tempo offset 3
 *
 * NOTE: The offset ratios are expressed as a numerator in the expression
 * n/64.  To get a ratio from this number, divide the number by 64.  To get
 * this number from a ratio, multiply the ratio by 64.
 * Examples:
 *   32 ==>  32/64 = 0.5
 *   63 ==>  63/64 = 0.9
 *   64 ==>  64/64 = 1.0
 *  128 ==> 128/64 = 2.0
 *
 * @return tempo offset 3
 */
int QWrk::getTempoOfs3() const
{
    return d->m_TempoOfs3;
}

/**
 * Auto-Punch enabled?
 * @return Auto-Punch enabled
 */
bool QWrk::getPunchEnabled() const
{
    return d->m_PunchEnabled;
}

/**
 * Punch-in time
 * @return punch-in time
 */
int QWrk::getPunchInTime() const
{
    return d->m_PunchInTime;
}

/**
 * Punch-out time
 * @return Punch-out time
 */
int QWrk::getPunchOutTime() const
{
    return d->m_PunchOutTime;
}

/**
 * Time of latest event (incl. all tracks)
 * @return Time of latest event
 */
int QWrk::getEndAllTime() const
{
    return d->m_EndAllTime;
}

/**
 * Gets a single byte from the stream
 * @return A Single byte
 */
quint8 QWrk::readByte()
{
    quint8 b = 0xff;
    if (!d->m_IOStream->atEnd())
        *d->m_IOStream >> b;
    return b;
}

/**
 * Converts two bytes into a single 16-bit value
 * @param c1 first byte
 * @param c2 second byte
 * @return 16-bit value
 */
quint16 QWrk::to16bit(quint8 c1, quint8 c2)
{
    quint16 value = (c1 << 8);
    value += c2;
    return value;
}

/**
 * Converts four bytes into a single 32-bit value
 * @param c1 1st byte
 * @param c2 2nd byte
 * @param c3 3rd byte
 * @param c4 4th byte
 * @return 32-bit value
 */
quint32 QWrk::to32bit(quint8 c1, quint8 c2, quint8 c3, quint8 c4)
{
    quint32 value = (c1 << 24);
    value += (c2 << 16);
    value += (c3 << 8);
    value += c4;
    return value;
}

/**
 * Reads a 16-bit value
 * @return 16-bit value
 */
quint16 QWrk::read16bit()
{
    quint8 c1, c2;
    c1 = readByte();
    c2 = readByte();
    return to16bit(c2, c1);
}

/**
 * Reads a 24-bit value
 * @return 32-bit value
 */
quint32 QWrk::read24bit()
{
    quint8 c1, c2, c3;
    c1 = readByte();
    c2 = readByte();
    c3 = readByte();
    return to32bit(0, c3, c2, c1);
}

/**
 * Reads a 32-bit value
 * @return 32-bit value
 */
quint32 QWrk::read32bit()
{
    quint8 c1, c2, c3, c4;
    c1 = readByte();
    c2 = readByte();
    c3 = readByte();
    c4 = readByte();
    return to32bit(c4, c3, c2, c1);
}

/**
 * Reads a string
 * @return a string
 */
QString QWrk::readString(int len)
{
    QString s;
    if ( len > 0 ) {
        quint8 c = 0xff;
        QByteArray data;
        for ( int i = 0; i < len && c != 0; ++i ) {
            c = readByte();
            if ( c != 0)
                data += c;
        }
        if (d->m_codec == NULL)
            s = QString(data);
        else
            s = d->m_codec->toUnicode(data);
    }
    return s;
}

/**
 * Reads a variable length string (C-style)
 * @return a string
 */
QString QWrk::readVarString()
{
    QString s;
    QByteArray data;
    quint8 b;
    do {
        b = readByte();
        if (b != 0)
            data += b;
    } while (b != 0);
    if (d->m_codec == NULL)
        s = QString(data);
    else
        s = d->m_codec->toUnicode(data);
    return s;
}

/**
 * Current position in the data stream
 * @return current position
 */
long QWrk::getFilePos()
{
    return d->m_IOStream->device()->pos();
}

/**
 * Seeks to a new position in the data stream
 * @param pos new position
 */
void QWrk::seek(qint64 pos)
{
    d->m_IOStream->device()->seek(pos);
}

/**
 * Checks if the data stream pointer has reached the end position
 * @return true if the read pointer is at end
 */
bool QWrk::atEnd()
{
    return d->m_IOStream->atEnd();
}

/**
 * Jumps the given size in the data stream
 * @param size the gap size
 */
void QWrk::readGap(int size)
{
    if ( size > 0)
        seek( getFilePos() + size );
}

/**
 * Reads a stream.
 * @param stream Pointer to an existing and opened stream
 */
void QWrk::readFromStream(QDataStream *stream)
{
    d->m_IOStream = stream;
    wrkRead();
}

/**
 * Reads a stream from a disk file.
 * @param fileName Name of an existing file.
 */
void QWrk::readFromFile(const QString& fileName)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QDataStream ds(&file);
    readFromStream(&ds);
    file.close();
}

void QWrk::processTrackChunk()
{
    int namelen;
    QString name[2];
    int trackno;
    int channel;
    int pitch;
    int velocity;
    int port;
    bool selected;
    bool muted;
    bool loop;

    trackno = read16bit();
    for(int i=0; i<2; ++i) {
        namelen = readByte();
        name[i] = readString(namelen);
    }
    channel = (qint8) readByte();
    pitch = readByte();
    velocity = readByte();
    port = readByte();
    quint8 flags = readByte();
    selected = ((flags & 1) != 0);
    muted = ((flags & 2) != 0);
    loop = ((flags & 4) != 0);
    Q_EMIT signalWRKTrack( name[0], name[1],
                           trackno, channel, pitch,
                           velocity, port, selected,
                           muted, loop );
}

void QWrk::processVarsChunk()
{
    d->m_Now = read32bit();
    d->m_From = read32bit();
    d->m_Thru = read32bit();
    d->m_KeySig = readByte();
    d->m_Clock = readByte();
    d->m_AutoSave = readByte();
    d->m_PlayDelay = readByte();
    readGap(1);
    d->m_ZeroCtrls = (readByte() != 0);
    d->m_SendSPP = (readByte() != 0);
    d->m_SendCont = (readByte() != 0);
    d->m_PatchSearch = (readByte() != 0);
    d->m_AutoStop = (readByte() != 0);
    d->m_StopTime = read32bit();
    d->m_AutoRewind = (readByte() != 0);
    d->m_RewindTime = read32bit();
    d->m_MetroPlay = (readByte() != 0);
    d->m_MetroRecord = (readByte() != 0);
    d->m_MetroAccent = (readByte() != 0);
    d->m_CountIn = readByte();
    readGap(2);
    d->m_ThruOn = (readByte() != 0);
    readGap(19);
    d->m_AutoRestart = (readByte() != 0);
    d->m_CurTempoOfs = readByte();
    d->m_TempoOfs1 = readByte();
    d->m_TempoOfs2 = readByte();
    d->m_TempoOfs3 = readByte();
    readGap(2);
    d->m_PunchEnabled = (readByte() != 0);
    d->m_PunchInTime = read32bit();
    d->m_PunchOutTime = read32bit();
    d->m_EndAllTime = read32bit();

    Q_EMIT signalWRKGlobalVars();
}

void QWrk::processTimebaseChunk()
{
    quint16 timebase = read16bit();
    d->m_division = timebase;
    Q_EMIT signalWRKTimeBase(timebase);
}

void QWrk::processNoteArray(int track, int events)
{
    quint32 time = 0;
    quint8  status = 0, data1 = 0, data2 = 0;
    quint16 dur = 0;
    int value = 0, type = 0, channel = 0, len = 0;
    QString text;
    QByteArray data;
    for ( int i = 0; i < events; ++i ) {
        time = read24bit();
        status = readByte();
        dur = 0;
        if (status >= 0x90) {
            type = status & 0xf0;
            channel = status & 0x0f;
            data1 = readByte();
            if (type == 0x90 || type == 0xA0  || type == 0xB0 || type == 0xE0)
                data2 = readByte();
            if (type == 0x90)
                dur = read16bit();
            switch (type) {
                case 0x90:
                    Q_EMIT signalWRKNote(track, time, channel, data1, data2, dur);
                    break;
                case 0xA0:
                    Q_EMIT signalWRKKeyPress(track, time, channel, data1, data2);
                    break;
                case 0xB0:
                    Q_EMIT signalWRKCtlChange(track, time, channel, data1, data2);
                    break;
                case 0xC0:
                    Q_EMIT signalWRKProgram(track, time, channel, data1);
                    break;
                case 0xD0:
                    Q_EMIT signalWRKChanPress(track, time, channel, data1);
                    break;
                case 0xE0:
                    value = (data2 << 7) + data1 - 8192;
                    Q_EMIT signalWRKPitchBend(track, time, channel, value);
                    break;
                case 0xF0:
                    Q_EMIT signalWRKSysexEvent(track, time, data1);
                    break;
            }
        } else if (status == 5) {
            int code = read16bit();
            len = read32bit();
            text = readString(len);
            Q_EMIT signalWRKExpression(track, time, code, text);
        } else if (status == 6) {
            int code = read16bit();
            dur = read16bit();
            readGap(4);
            Q_EMIT signalWRKHairpin(track, time, code, dur);
        } else if (status == 7) {
            len = read32bit();
            text = readString(len);
            data.clear();
            for(int j=0; j<13; ++j) {
                int byte = readByte();
                data += byte;
            }
            Q_EMIT signalWRKChord(track, time, text, data);
        } else if (status == 8) {
            len = read16bit();
            data.clear();
            for(int j=0; j<len; ++j) {
                int byte = readByte();
                data += byte;
            }
            Q_EMIT signalWRKSysex(0, QString(), false, 0, data);
        } else {
            len = read32bit();
            text = readString(len);
            Q_EMIT signalWRKText(track, time, status, text);
        }
    }
    Q_EMIT signalWRKStreamEnd(time + dur);
}

void QWrk::processStreamChunk()
{
    long time = 0;
    int dur = 0, value = 0, type = 0, channel = 0;
    quint8 status = 0, data1 = 0, data2 = 0;
    quint16 track = read16bit();
    int events = read16bit();
    for ( int i = 0; i < events; ++i ) {
        time = read24bit();
        status = readByte();
        data1 = readByte();
        data2 = readByte();
        dur = read16bit();
        type = status & 0xf0;
        channel = status & 0x0f;
        switch (type) {
            case 0x90:
                Q_EMIT signalWRKNote(track, time, channel, data1, data2, dur);
                break;
            case 0xA0:
                Q_EMIT signalWRKKeyPress(track, time, channel, data1, data2);
                break;
            case 0xB0:
                Q_EMIT signalWRKCtlChange(track, time, channel, data1, data2);
                break;
            case 0xC0:
                Q_EMIT signalWRKProgram(track, time, channel, data1);
                break;
            case 0xD0:
                Q_EMIT signalWRKChanPress(track, time, channel, data1);
                break;
            case 0xE0:
                value = (data2 << 7) + data1 - 8192;
                Q_EMIT signalWRKPitchBend(track, time, channel, value);
                break;
            case 0xF0:
                Q_EMIT signalWRKSysexEvent(track, time, data1);
                break;
        }
    }
    Q_EMIT signalWRKStreamEnd(time + dur);
}

void QWrk::processMeterChunk()
{
    int count = read16bit();
    for (int i = 0; i < count; ++i) {
        readGap(4);
        int measure = read16bit();
        int  num = readByte();
        int  den = pow(2, readByte());
        readGap(4);
        Q_EMIT signalWRKTimeSig(measure, num, den);
    }
}

void QWrk::processMeterKeyChunk()
{
    int count = read16bit();
    for (int i = 0; i < count; ++i) {
        int measure = read16bit();
        int  num = readByte();
        int  den = pow(2, readByte());
        qint8 alt = readByte();
        Q_EMIT signalWRKTimeSig(measure, num, den);
        Q_EMIT signalWRKKeySig(measure, alt);
    }
}

double QWrk::getRealTime(long ticks) const
{
    double division = 1.0 * d->m_division;
    RecTempo last;
    last.time = 0;
    last.tempo = 100.0;
    last.seconds = 0.0;
    if (!d->m_tempos.isEmpty()) {
        foreach(const RecTempo& rec, d->m_tempos) {
            if (rec.time >= ticks)
                break;
            last = rec;
        }
    }
    return last.seconds + (((ticks - last.time) / division) * (60.0 / last.tempo));
}

void QWrk::processTempoChunk(int factor)
{
    double division = 1.0 * d->m_division;
    int count = read16bit();
    RecTempo last, next;
    for (int i = 0; i < count; ++i) {

        long time = read32bit();
        readGap(4);
        long tempo = read16bit() * factor;
        readGap(8);

        next.time = time;
        next.tempo = tempo / 100.0;
        next.seconds = 0.0;
        last.time = 0;
        last.tempo = next.tempo;
        last.seconds = 0.0;
        if (! d->m_tempos.isEmpty()) {
            foreach(const RecTempo& rec, d->m_tempos) {
                if (rec.time >= time)
                    break;
                last = rec;
            }
            next.seconds = last.seconds +
                (((time - last.time) / division) * (60.0 / last.tempo));
        }
        d->m_tempos.append(next);

        Q_EMIT signalWRKTempo(time, tempo);
    }
}

void QWrk::processSysexChunk()
{
    int j;
    QString name;
    QByteArray data;
    int bank = readByte();
    int length = read16bit();
    bool autosend = (readByte() != 0);
    int namelen = readByte();
    name = readString(namelen);
    for(j=0; j<length; ++j) {
        int byte = readByte();
        data += byte;
    }
    Q_EMIT signalWRKSysex(bank, name, autosend, 0, data);
}

void QWrk::processSysex2Chunk()
{
    int j;
    QString name;
    QByteArray data;
    int bank = read16bit();
    int length = read32bit();
    quint8 b = readByte();
    int port = ( b & 0xf0 ) >> 4;
    bool autosend = ( (b & 0x0f) != 0);
    int namelen = readByte();
    name = readString(namelen);
    for(j=0; j<length; ++j) {
        int byte = readByte();
        data += byte;
    }
    Q_EMIT signalWRKSysex(bank, name, autosend, port, data);
}

void QWrk::processNewSysexChunk()
{
    int j;
    QString name;
    QByteArray data;
    int bank = read16bit();
    int length = read32bit();
    int port = read16bit();
    bool autosend = (readByte() != 0);
    int namelen = readByte();
    name = readString(namelen);
    for(j=0; j<length; ++j) {
        int byte = readByte();
        data += byte;
    }
    Q_EMIT signalWRKSysex(bank, name, autosend, port, data);
}

void QWrk::processThruChunk()
{
    readGap(2);
    qint8 port = readByte();    // 0->127
    qint8 channel = readByte(); // -1, 0->15
    qint8 keyPlus = readByte(); // 0->127
    qint8 velPlus = readByte(); // 0->127
    qint8 localPort = readByte();
    qint8 mode = readByte();
    Q_EMIT signalWRKThru(mode, port, channel, keyPlus, velPlus, localPort);
}

void QWrk::processTrackOffset()
{
    quint16 track = read16bit();
    qint16 offset = read16bit();
    Q_EMIT signalWRKTrackOffset(track, offset);
}

void QWrk::processTrackReps()
{
    quint16 track = read16bit();
    quint16 reps = read16bit();
    Q_EMIT signalWRKTrackReps(track, reps);
}

void QWrk::processTrackPatch()
{
    quint16 track = read16bit();
    qint8 patch = readByte();
    Q_EMIT signalWRKTrackPatch(track, patch);
}

void QWrk::processTimeFormat()
{
    quint16 fmt = read16bit();
    quint16 ofs = read16bit();
    Q_EMIT signalWRKTimeFormat(fmt, ofs);
}

void QWrk::processComments()
{
    int len = read16bit();
    QString text = readString(len);
    Q_EMIT signalWRKComments(text);
}

void QWrk::processVariableRecord(int max)
{
    int datalen = max - 32;
    QByteArray data;
    QString name = readVarString();
    readGap(31 - name.length());
    for ( int i = 0; i < datalen; ++i )
        data += readByte();
    Q_EMIT signalWRKVariableRecord(name, data);
}

void QWrk::processUnknown(int id)
{
    Q_EMIT signalWRKUnknownChunk(id, d->m_lastChunkData);
}

void QWrk::processNewTrack()
{
    qint16 bank = -1;
    qint16 patch = -1;
    qint16 vol = -1;
    qint16 pan = -1;
    qint8 key = -1;
    qint8 vel = 0;
    quint8 port = 0;
    qint8 channel = 0;
    bool selected = false;
    bool muted = false;
    bool loop = false;
    quint16 track = read16bit();
    quint8 len = readByte();
    QString name = readString(len);
    bank = read16bit();
    patch = read16bit();
    vol = read16bit();
    pan = read16bit();
    key = readByte();
    vel = readByte();
    readGap(7);
    port = readByte();
    channel = readByte();
    muted = (readByte() != 0);
    Q_EMIT signalWRKNewTrack(name, track, channel, key, vel, port, selected, muted, loop);
    if (bank > -1)
        Q_EMIT signalWRKTrackBank(track, bank);
    if (patch > -1) {
        if (channel > -1)
            Q_EMIT signalWRKProgram(track, 0, channel, patch);
        else
            Q_EMIT signalWRKTrackPatch(track, patch);
    }
}

void QWrk::processSoftVer()
{
    int len = readByte();
    QString vers = readString(len);
    Q_EMIT signalWRKSoftVer(vers);
}

void QWrk::processTrackName()
{
    int track = read16bit();
    int len = readByte();
    QString name = readString(len);
    Q_EMIT signalWRKTrackName(track, name);
}

void QWrk::processStringTable()
{
    QStringList table;
    int rows = read16bit();
    for (int i = 0; i < rows; ++i) {
        int len = readByte();
        QString name = readString(len);
        int idx = readByte();
        table.insert(idx, name);
    }
    Q_EMIT signalWRKStringTable(table);
}

void QWrk::processLyricsStream()
{
    quint16 track = read16bit();
    int events = read32bit();
    processNoteArray(track, events);
}

void QWrk::processTrackVol()
{
    quint16 track = read16bit();
    int vol = read16bit();
    Q_EMIT signalWRKTrackVol(track, vol);
}

void QWrk::processNewTrackOffset()
{
    quint16 track = read16bit();
    int offset = read32bit();
    Q_EMIT signalWRKTrackOffset(track, offset);
}

void QWrk::processTrackBank()
{
    quint16 track = read16bit();
    int bank = read16bit();
    Q_EMIT signalWRKTrackBank(track, bank);
}

void QWrk::processSegmentChunk()
{
    QString name;
    int track = read16bit();
    int offset = read32bit();
    readGap(8);
    int len = readByte();
    name = readString(len);
    readGap(20);
    Q_EMIT signalWRKSegment(track, offset, name);
    int events = read32bit();
    processNoteArray(track, events);
}

void QWrk::processNewStream()
{
    QString name;
    int track = read16bit();
    int len = readByte();
    name = readString(len);
    Q_EMIT signalWRKSegment(track, 0, name);
    int events = read32bit();
    processNoteArray(track, events);
}

void QWrk::processEndChunk()
{
    emit signalWRKEnd();
}

int QWrk::readChunk()
{
    long start_pos, final_pos;
    int ck_len, ck = readByte();
    if (ck != END_CHUNK) {
        ck_len = read32bit();
        start_pos = getFilePos();
        final_pos = start_pos + ck_len;
        readRawData(ck_len);
        seek(start_pos);
        switch (ck) {
        case TRACK_CHUNK:
            processTrackChunk();
            break;
        case VARS_CHUNK:
            processVarsChunk();
            break;
        case TIMEBASE_CHUNK:
            processTimebaseChunk();
            break;
        case STREAM_CHUNK:
            processStreamChunk();
            break;
        case METER_CHUNK:
            processMeterChunk();
            break;
        case TEMPO_CHUNK:
            processTempoChunk(100);
            break;
        case NTEMPO_CHUNK:
            processTempoChunk();
            break;
        case SYSEX_CHUNK:
            processSysexChunk();
            break;
        case THRU_CHUNK:
            processThruChunk();
            break;
        case TRKOFFS_CHUNK:
            processTrackOffset();
            break;
        case TRKREPS_CHUNK:
            processTrackReps();
            break;
        case TRKPATCH_CHUNK:
            processTrackPatch();
            break;
        case TIMEFMT_CHUNK:
            processTimeFormat();
            break;
        case COMMENTS_CHUNK:
            processComments();
            break;
        case VARIABLE_CHUNK:
            processVariableRecord(ck_len);
            break;
        case NTRACK_CHUNK:
            processNewTrack();
            break;
        case SOFTVER_CHUNK:
            processSoftVer();
            break;
        case TRKNAME_CHUNK:
            processTrackName();
            break;
        case STRTAB_CHUNK:
            processStringTable();
            break;
        case LYRICS_CHUNK:
            processLyricsStream();
            break;
        case TRKVOL_CHUNK:
            processTrackVol();
            break;
        case NTRKOFS_CHUNK:
            processNewTrackOffset();
            break;
        case TRKBANK_CHUNK:
            processTrackBank();
            break;
        case METERKEY_CHUNK:
            processMeterKeyChunk();
            break;
        case SYSEX2_CHUNK:
            processSysex2Chunk();
            break;
        case NSYSEX_CHUNK:
            processNewSysexChunk();
            break;
        case SGMNT_CHUNK:
            processSegmentChunk();
            break;
        case NSTREAM_CHUNK:
            processNewStream();
            break;
        default:
            processUnknown(ck);
        }
        seek(final_pos);
    }
    return ck;
}

void QWrk::wrkRead()
{
    int vma, vme;
    int ck_id;
    QByteArray hdr(HEADER.length(), ' ');
    d->m_tempos.clear();
    d->m_IOStream->device()->read(hdr.data(), HEADER.length());
    if (hdr == HEADER) {
        readGap(1);
        vme = readByte();
        vma = readByte();
        Q_EMIT signalWRKHeader(vma, vme);
        do {
            ck_id = readChunk();
        }  while (ck_id != END_CHUNK);
        if (!atEnd())
            Q_EMIT signalWRKError("Corrupted file");
        else
            processEndChunk();
    } else
        Q_EMIT signalWRKError("Invalid file format");
}

} // namespace drumstick
