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

#include "qsmf.h"
#include <limits>
#include <QList>
#include <QFile>
#include <QDataStream>
#include <QTextCodec>

/**
 * @file qsmf.cpp
 * Implementation of a class managing Standard MIDI Files input/output
 */

namespace drumstick {

/**
 * @addtogroup SMF
 * @{
 *
 * QSmf provides a mechanism to parse and encode Standard MIDI Files, without
 * the burden of a policy forcing to use some internal sequence representation.
 *
 * This class is not related or based on the ALSA library.
 *
 * @}
 */

class QSmf::QSmfPrivate {
public:
    QSmfPrivate():
        m_Interactive(false),
        m_CurrTime(0),
        m_RealTime(0),
        m_DblRealTime(0),
        m_DblOldRealtime(0),
        m_Division(96),
        m_CurrTempo(500000),
        m_OldCurrTempo(500000),
        m_OldRealTime(0),
        m_OldCurrTime(0),
        m_RevisedTime(0),
        m_TempoChangeTime(0),
        m_ToBeRead(0),
        m_NumBytesWritten(0),
        m_Tracks(0),
        m_fileFormat(0),
        m_LastStatus(0),
        m_codec(0),
        m_IOStream(0)
    { }

    bool m_Interactive;     /**< file and track headers are not required */
    quint64 m_CurrTime;     /**< current time in delta-time units */
    quint64 m_RealTime;     /**< current time in 1/16 centisecond-time units */
    double m_DblRealTime;   /**< as above, floating */
    double m_DblOldRealtime;
    int m_Division;         /**< ticks per beat. Default = 96 */
    quint64 m_CurrTempo;    /**< microseconds per quarter note */
    quint64 m_OldCurrTempo;
    quint64 m_OldRealTime;
    quint64 m_OldCurrTime;
    quint64 m_RevisedTime;
    quint64 m_TempoChangeTime;
    quint64 m_ToBeRead;
    quint64 m_NumBytesWritten;
    int m_Tracks;
    int m_fileFormat;
    int m_LastStatus;
    QTextCodec *m_codec;
    QDataStream *m_IOStream;
    QByteArray m_MsgBuff;
    QList<QSmfRecTempo> m_TempoList;
};

/**
 * Constructor
 * @param parent Optional parent object
 */
QSmf::QSmf(QObject * parent) :
    QObject(parent),
    d(new QSmfPrivate)
{ }

/**
 * Destructor
 */
QSmf::~QSmf()
{
    d->m_TempoList.clear();
    delete d;
}

/**
 * Check if the SMF stream is positioned at the end.
 * @return True if the SMF stream is at the end
 */
bool QSmf::endOfSmf()
{
    return d->m_IOStream->atEnd();
}

/**
 * Gets a single byte from the SMF stream
 * @return A Single byte
 */
quint8 QSmf::getByte()
{
    quint8 b = 0;
    if (!d->m_IOStream->atEnd())
    {
        *d->m_IOStream >> b;
        d->m_ToBeRead--;
    }
    return b;
}

/**
 * Puts a single byte to the SMF stream
 * @param value A Single byte
 */
void QSmf::putByte(quint8 value)
{
    *d->m_IOStream << value;
    d->m_NumBytesWritten++;
}

/**
 * Adds a tempo change to the internal tempo list
 * @param tempo Tempo in microseconds per quarter
 * @param time Location in ticks
 */
void QSmf::addTempo(quint64 tempo, quint64 time)
{
    QSmfRecTempo tempoRec;
    tempoRec.tempo = tempo;
    tempoRec.time = time;
    d->m_TempoList.append(tempoRec);
}

/**
 * Reads a SMF header
 */
void QSmf::readHeader()
{
    d->m_CurrTime = 0;
    d->m_RealTime = 0;
    d->m_Division = 96;
    d->m_CurrTempo = 500000;
    d->m_OldCurrTempo = 500000;
    addTempo(d->m_CurrTempo, 0);
    if (d->m_Interactive)
    {
        d->m_fileFormat= 0;
        d->m_Tracks = 1;
        d->m_Division = 96;
    }
    else
    {
        readExpected("MThd");
        d->m_ToBeRead = read32bit();
        d->m_fileFormat = read16bit();
        d->m_Tracks = read16bit();
        d->m_Division = read16bit();
    }
    emit signalSMFHeader(d->m_fileFormat, d->m_Tracks, d->m_Division);

    /* flush any extra stuff, in case the length of header is not */
    while ((d->m_ToBeRead > 0) && !endOfSmf())
    {
        getByte();
    }
}

/**
 * Reads a track chunk
 */
void QSmf::readTrack()
{
    /* This array is indexed by the high half of a status byte.  It's
     value is either the number of bytes needed (1 or 2) for a channel
     message, or 0 (meaning it's not  a channel message). */
    static const quint8 chantype[16] =
        { 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 1, 1, 2, 0 };

    quint64 lookfor;
    quint8 c, c1, type;
    bool sysexcontinue; // 1 if last message was an unfinished SysEx
    bool running; // 1 when running status used
    quint8 status; // status value (e.g. 0x90==note-on)
    int needed;
    double delta_secs;
    quint64 delta_ticks, save_time, save_tempo;

    sysexcontinue = false;
    status = 0;
    if (d->m_Interactive)
    {
        d->m_ToBeRead = std::numeric_limits<unsigned long long>::max();
    }
    else
    {
        readExpected("MTrk");
        d->m_ToBeRead = read32bit();
    }
    d->m_CurrTime = 0;
    d->m_RealTime = 0;
    d->m_DblRealTime = 0;
    d->m_DblOldRealtime = 0;
    d->m_OldCurrTime = 0;
    d->m_OldRealTime = 0;
    d->m_CurrTempo = findTempo();

    emit signalSMFTrackStart();

    while (!endOfSmf() && (d->m_Interactive || d->m_ToBeRead > 0))
    {
        if (d->m_Interactive)
        {
            d->m_CurrTime++;
        }
        else
        {
            delta_ticks = readVarLen();
            d->m_RevisedTime = d->m_CurrTime;
            d->m_CurrTime += delta_ticks;
            while (d->m_RevisedTime < d->m_CurrTime)
            {
                save_time = d->m_RevisedTime;
                save_tempo = d->m_CurrTempo;
                d->m_CurrTempo = findTempo();
                if (d->m_CurrTempo != d->m_OldCurrTempo)
                {
                    d->m_OldCurrTempo = d->m_CurrTempo;
                    d->m_OldRealTime = d->m_RealTime;
                    if (d->m_RevisedTime != d->m_TempoChangeTime)
                    {
                        d->m_DblOldRealtime = d->m_DblRealTime;
                        d->m_OldCurrTime = save_time;
                    }
                    delta_secs = ticksToSecs(d->m_RevisedTime - d->m_OldCurrTime,
                            d->m_Division, save_tempo);
                    d->m_DblRealTime = d->m_DblOldRealtime + delta_secs * 1600.0;
                    d->m_RealTime = static_cast<quint64>(0.5 + d->m_DblRealTime);
                    if (d->m_RevisedTime == d->m_TempoChangeTime)
                    {
                        d->m_OldCurrTime = d->m_RevisedTime;
                        d->m_DblOldRealtime = d->m_DblRealTime;
                    }
                }
                else
                {
                    delta_secs = ticksToSecs(d->m_RevisedTime - d->m_OldCurrTime,
                            d->m_Division, d->m_CurrTempo);
                    d->m_DblRealTime = d->m_DblOldRealtime + delta_secs * 1600.0;
                    d->m_RealTime = static_cast<quint64>(0.5 + d->m_DblRealTime);
                }
            }
        }

        c = getByte();
        if (sysexcontinue && (c != end_of_sysex))
        {
            SMFError("didn't find expected continuation of a SysEx");
        }
        if (c < 0xf8)
        {
            if ((c & 0x80) == 0)
            {
                if (status == 0)
                {
                    SMFError("unexpected running status");
                }
                running = true;
            }
            else
            {
                status = c;
                running = false;
            }
            needed = chantype[status >> 4 & 0x0f];
            if (needed != 0)
            {
                if (running)
                {
                    c1 = c;
                }
                else
                {
                    c1 = getByte();
                }
                if (needed > 1)
                {
                    channelMessage(status, c1, getByte());
                }
                else
                {
                    channelMessage(status, c1, 0);
                }
                continue;
            }
        }

        switch (c)
        {
        case meta_event:
            type = getByte();
            lookfor = readVarLen();
            lookfor = d->m_ToBeRead - lookfor;
            msgInit();
            while (d->m_ToBeRead > lookfor)
            {
                msgAdd(getByte());
            }
            metaEvent(type);
            break;
        case system_exclusive:
            lookfor = readVarLen();
            lookfor = d->m_ToBeRead - lookfor;
            msgInit();
            msgAdd(system_exclusive);
            while (d->m_ToBeRead > lookfor)
            {
                c = getByte();
                msgAdd(c);
            }
            if (c == end_of_sysex)
            {
                sysEx();
            }
            else
            {
                sysexcontinue = true;
            }
            break;
        case end_of_sysex:
            lookfor = readVarLen();
            lookfor = d->m_ToBeRead - lookfor;
            if (!sysexcontinue)
            {
                msgInit();
            }
            while (d->m_ToBeRead > lookfor)
            {
                c = getByte();
                msgAdd(c);
            }
            if (sysexcontinue)
            {
                if (c == end_of_sysex)
                {
                    sysEx();
                    sysexcontinue = false;
                }
            }
            break;
        default:
            badByte(c, d->m_IOStream->device()->pos() - 1);
            break;
        }
    }
    emit signalSMFTrackEnd();
}

/**
 * Reads a SMF stream.
 */
void QSmf::SMFRead()
{
    int i;
    readHeader();
    for ( i = d->m_Tracks; (i > 0) && !endOfSmf(); i--)
    {
        readTrack();
    }
}

/**
 * Writes a SMF stream.
 *
 * Every MIDI file starts with a header.
 * In format 1 files, the first track is a tempo map.
 * The rest of the file is a series of tracks
 */
void QSmf::SMFWrite()
{
    int i;
    d->m_LastStatus = 0;
    writeHeaderChunk(d->m_fileFormat, d->m_Tracks, d->m_Division);
    d->m_LastStatus = 0;
    if (d->m_fileFormat == 1)
    {
        emit signalSMFWriteTempoTrack();
    }
    for (i = 0; i < d->m_Tracks; ++i)
    {
        writeTrackChunk(i);
    }
}

/**
 * Reads a SMF stream.
 * @param stream Pointer to an existing and opened stream
 */
void QSmf::readFromStream(QDataStream *stream)
{
    d->m_IOStream = stream;
    SMFRead();
}

/**
 * Reads a SMF stream from a disk file.
 * @param fileName Name of an existing file.
 */
void QSmf::readFromFile(const QString& fileName)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QDataStream ds(&file);
    readFromStream(&ds);
    file.close();
}

/**
 * Writes a SMF stream
 * @param stream Pointer to an existing and opened stream
 */
void QSmf::writeToStream(QDataStream *stream)
{
    d->m_IOStream = stream;
    SMFWrite();
}

/**
 * Writes a SMF stream to a disk file
 * @param fileName File name
 */
void QSmf::writeToFile(const QString& fileName)
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    QDataStream ds(&file);
    writeToStream(&ds);
    file.close();
}

/**
 * Writes a SMF header chuck
 * @param format SMF Format (0/1/2)
 * @param ntracks Number of tracks
 * @param division Resolution in ticks per quarter note
 */
void QSmf::writeHeaderChunk(int format, int ntracks, int division)
{
    write32bit(MThd);
    write32bit(6);
    write16bit(format);
    write16bit(ntracks);
    write16bit(division);
}

/**
 * Writes a track chuck
 * @param track Number of the track
 */
void QSmf::writeTrackChunk(int track)
{
    quint32 trkhdr;
    quint32 trklength;
    qint64 offset;
    qint64 place_marker;

    d->m_LastStatus = 0;
    trkhdr = MTrk;
    trklength = 0;
    offset = d->m_IOStream->device()->pos();
    write32bit(trkhdr);
    write32bit(trklength);
    d->m_NumBytesWritten = 0;

    emit signalSMFWriteTrack(track);

    place_marker = d->m_IOStream->device()->pos();
    d->m_IOStream->device()->seek(offset);
    trklength = d->m_NumBytesWritten;
    write32bit(trkhdr);
    write32bit(trklength);
    d->m_IOStream->device()->seek(place_marker);
}

/**
 * Writes a variable length Meta Event
 * @param deltaTime Time offset in ticks
 * @param type Meta event type
 * @param data Message data
 */
void QSmf::writeMetaEvent(long deltaTime, int type, const QByteArray& data)
{
    writeVarLen(deltaTime);
    d->m_LastStatus = meta_event;
    putByte(d->m_LastStatus);
    putByte(type);
    writeVarLen(data.size());
    foreach(char byte, data)
        putByte(byte);
}

/**
 * Writes a Text Meta Event
 * @param deltaTime Time offset in ticks
 * @param type Meta event type
 * @param data Message text
 */
void QSmf::writeMetaEvent(long deltaTime, int type, const QString& data)
{
    writeVarLen(deltaTime);
    putByte(d->m_LastStatus = meta_event);
    putByte(type);
    QByteArray lcldata;
    if (d->m_codec == NULL)
        lcldata = data.toLatin1();
    else
        lcldata = d->m_codec->fromUnicode(data);
    writeVarLen(lcldata.length());
    foreach(char byte, lcldata)
        putByte(byte);
}

/**
 * Writes a simple Meta event
 * @param deltaTime Time offset in ticks
 * @param type Meta event type
 * @param data Meta event data
 * @since 0.2.0
 */
void QSmf::writeMetaEvent(long deltaTime, int type, int data)
{
    writeVarLen(deltaTime);
    putByte(d->m_LastStatus = meta_event);
    putByte(type);
    putByte(1);
    putByte(data);
}

/**
 * Writes a simple Meta event
 * @param deltaTime Time offset in ticks
 * @param type Meta event type
 */
void QSmf::writeMetaEvent(long deltaTime, int type)
{
    writeVarLen(deltaTime);
    putByte(d->m_LastStatus = meta_event);
    putByte(type);
    putByte(0);
}

/**
 * Writes a variable length MIDI message
 * @param deltaTime Time offset in ticks
 * @param type MIDI event type
 * @param chan MIDI Channel
 * @param data Message data
 */
void QSmf::writeMidiEvent(long deltaTime, int type, int chan,
                          const QByteArray& data)
{
    int i, j, size;
    quint8 c;
    writeVarLen(deltaTime);
    if ((type == system_exclusive) || (type == end_of_sysex))
    {
        c = type;
        d->m_LastStatus = 0;
    }
    else
    {
        if (chan > 15)
        {
            SMFError("error: MIDI channel greater than 16");
        }
        c = type | chan;
    }
    if (d->m_LastStatus != c)
    {
        d->m_LastStatus = c;
        putByte(c);
    }
    if (type == system_exclusive || type == end_of_sysex)
    {
        size = data.size();
        if (data[0] == type)
            --size;
        writeVarLen(size);
    }
    j = (data[0] == type ? 1 : 0);
    for (i = j; i < data.size(); ++i)
    {
        putByte(data[i]);
    }
}

/**
 * Writes a MIDI message with a single parameter
 * @param deltaTime Time offset in ticks
 * @param type MIDI event type
 * @param chan MIDI Channel
 * @param b1 Message parameter
 */
void QSmf::writeMidiEvent(long deltaTime, int type, int chan, int b1)
{
    quint8 c;
    writeVarLen(deltaTime);
    if ((type == system_exclusive) || (type == end_of_sysex))
    {
        SMFError("error: Wrong method for a system exclusive event");
    }
    if (chan > 15)
    {
        SMFError("error: MIDI channel greater than 16");
    }
    c = type | chan;
    if (d->m_LastStatus != c)
    {
        d->m_LastStatus = c;
        putByte(c);
    }
    putByte(b1);
}

/**
 * Writes a MIDI message with two parameters
 * @param deltaTime Time offset in ticks
 * @param type MIDI event type
 * @param chan MIDI Channel
 * @param b1 Message parameter 1
 * @param b2 Message parameter 2
 */
void QSmf::writeMidiEvent(long deltaTime, int type, int chan, int b1, int b2)
{
    quint8 c;
    writeVarLen(deltaTime);
    if ((type == system_exclusive) || (type == end_of_sysex))
    {
        SMFError("error: Wrong method for a system exclusive event");
    }
    if (chan > 15)
    {
        SMFError("error: MIDI channel greater than 16");
    }
    c = type | chan;
    if (d->m_LastStatus != c)
    {
        d->m_LastStatus = c;
        putByte(c);
    }
    putByte(b1);
    putByte(b2);
}

/**
 * Writes a variable length MIDI message
 * @param deltaTime Time offset in ticks
 * @param type MIDI event type
 * @param len  Message length
 * @param data Message data
 */
void QSmf::writeMidiEvent(long deltaTime, int type, long len, char* data)
{
    unsigned int i, j, size;
    quint8 c;
    writeVarLen(deltaTime);
    if ((type != system_exclusive) && (type != end_of_sysex))
    {
        SMFError("error: type should be system exclusive");
    }
    d->m_LastStatus = 0;
    c = type;
    putByte(c);
    size = len;
    c = (unsigned)data[0];
    if (c == type)
        --size;
    writeVarLen(size);
    j = (c == type ? 1 : 0);
    for (i = j; i < (unsigned)len; ++i)
    {
        putByte(data[i]);
    }
}

/**
 * Writes a MIDI Sequence number
 * @param deltaTime Time offset in ticks
 * @param seqnum Sequence number
 */
void QSmf::writeSequenceNumber(long deltaTime, int seqnum)
{
    writeVarLen(deltaTime);
    d->m_LastStatus = meta_event;
    putByte(d->m_LastStatus);
    putByte(sequence_number);
    putByte(2);
    putByte((seqnum >> 8) & 0xff);
    putByte(seqnum & 0xff);
}

/**
 * Writes a Tempo change message
 * @param deltaTime Time offset in ticks
 * @param tempo Tempo in microseconds per quarter note
 */
void QSmf::writeTempo(long deltaTime, long tempo)
{
    writeVarLen(deltaTime);
    putByte(d->m_LastStatus = meta_event);
    putByte(set_tempo);
    putByte(3);
    putByte((tempo >> 16) & 0xff);
    putByte((tempo >> 8) & 0xff);
    putByte(tempo & 0xff);
}

/**
 * Writes a Tempo change message
 * @param deltaTime Time offset in ticks
 * @param tempo Tempo  expressed in quarter notes per minute
 */
void QSmf::writeBpmTempo(long deltaTime, int tempo)
{
    long us_tempo = 60000000l / tempo;
    writeTempo(deltaTime, us_tempo);
}

/**
 * Writes a Time Signature message
 * @param deltaTime Time offset in ticks
 * @param num Numerator
 * @param den Denominator (exponent for a power of two)
 * @param cc Number of MIDI clocks in a metronome click
 * @param bb Number of notated 32nd notes in 24 MIDI clocks
 */
void QSmf::writeTimeSignature(long deltaTime, int num, int den, int cc, int bb)
{
    writeVarLen(deltaTime);
    putByte(d->m_LastStatus = meta_event);
    putByte(time_signature);
    putByte(4);
    putByte(num & 0xff);
    putByte(den & 0xff);
    putByte(cc & 0xff);
    putByte(bb & 0xff);
}

/**
 * Writes a key Signature message
 * @param deltaTime Time offset in ticks
 * @param tone Number of alterations (positive=sharps, negative=flats)
 * @param mode Scale mode (0=major, 1=minor)
 */
void QSmf::writeKeySignature(long deltaTime, int tone, int mode)
{
    writeVarLen(deltaTime);
    putByte(d->m_LastStatus = meta_event);
    putByte(key_signature);
    putByte(2);
    putByte((char)tone);
    putByte(mode & 0x01);
}

/**
 * Writes multi-length bytes
 * @param value Integer value
 */
void QSmf::writeVarLen(quint64 value)
{
    quint64 buffer;

    buffer = value & 0x7f;
    while ((value >>= 7) > 0)
    {
        buffer <<= 8;
        buffer |= 0x80;
        buffer += (value & 0x7f);
    }
    while (true)
    {
        putByte(buffer & 0xff);
        if (buffer & 0x80)
            buffer >>= 8;
        else
            break;
    }
}

/* These routines are used to make sure that the byte order of
 the various data types remains constant between machines. */
void QSmf::write32bit(quint32 data)
{
    putByte((data >> 24) & 0xff);
    putByte((data >> 16) & 0xff);
    putByte((data >> 8) & 0xff);
    putByte(data & 0xff);
}

void QSmf::write16bit(quint16 data)
{
    putByte((data >> 8) & 0xff);
    putByte(data & 0xff);
}

quint16 QSmf::to16bit(quint8 c1, quint8 c2)
{
    quint16 value;
    value = (c1 << 8);
    value += c2;
    return value;
}

quint32 QSmf::to32bit(quint8 c1, quint8 c2, quint8 c3, quint8 c4)
{
    quint32 value;
    value = (c1 << 24);
    value += (c2 << 16);
    value += (c3 << 8);
    value += c4;
    return value;
}

quint16 QSmf::read16bit()
{
    quint8 c1, c2;
    c1 = getByte();
    c2 = getByte();
    return to16bit(c1, c2);
}

quint32 QSmf::read32bit()
{
    quint8 c1, c2, c3, c4;
    c1 = getByte();
    c2 = getByte();
    c3 = getByte();
    c4 = getByte();
    return to32bit(c1, c2, c3, c4);
}

long QSmf::readVarLen()
{
    long value;
    quint8 c;

    c = getByte();
    value = c;
    if ((c & 0x80) != 0)
    {
        value &= 0x7f;
        do
        {
            c = getByte();
            value = (value << 7) + (c & 0x7f);
        } while ((c & 0x80) != 0);
    }
    return value;
}

void QSmf::readExpected(const QString& s)
{
    int j;
    quint8 b;
    for (j = 0; j < s.length(); ++j)
    {
        b = getByte();
        if (QChar(b) != s[j])
        {
            SMFError(QString("Invalid (%1) SMF format at %2").arg(b, 0, 16).arg(d->m_IOStream->device()->pos()));
            break;
        }
    }
}

quint64 QSmf::findTempo()
{
    quint64 result, old_tempo, new_tempo;
    QSmfRecTempo rec = d->m_TempoList.last();
    old_tempo = d->m_CurrTempo;
    new_tempo = d->m_CurrTempo;
    QList<QSmfRecTempo>::Iterator it;
    for( it = d->m_TempoList.begin(); it != d->m_TempoList.end(); ++it )
    {
        rec = (*it);
        if (rec.time <= d->m_CurrTime)
        {
            old_tempo = rec.tempo;
        }
        new_tempo = rec.tempo;
        if (rec.time > d->m_RevisedTime)
        {
            break;
        }
    }
    if ((rec.time <= d->m_RevisedTime) || (rec.time > d->m_CurrTime))
    {
        d->m_RevisedTime = d->m_CurrTime;
        result = old_tempo;
    }
    else
    {
        d->m_RevisedTime = rec.time;
        d->m_TempoChangeTime = d->m_RevisedTime;
        result = new_tempo;
    }
    return result;
}

/* This routine converts delta times in ticks into seconds. The
 else statement is needed because the formula is different for tracks
 based on notes and tracks based on SMPTE times. */
double QSmf::ticksToSecs(quint64 ticks, quint16 division, quint64 tempo)
{
    double result;
    double smpte_format;
    double smpte_resolution;

    if (division > 0)
    {
        result = static_cast<double>(ticks * tempo)/(division * 1000000.0);
    }
    else
    {
        smpte_format = upperByte(division);
        smpte_resolution = lowerByte(division);
        result = static_cast<double>(ticks)/(smpte_format * smpte_resolution
                * 1000000.0);
    }
    return result;
}

void QSmf::SMFError(const QString& s)
{
    emit signalSMFError(s);
}

void QSmf::channelMessage(quint8 status, quint8 c1, quint8 c2)
{
    quint8 chan;
    int k;
    chan = status & midi_channel_mask;
    if (c1 > 127)
    {
        SMFError(QString("ChannelMessage with bad c1 = %1").arg(c1));
        //c1 &= 127;
    }
    if (c2 > 127)
    {
        SMFError(QString("ChannelMessage with bad c2 = %1").arg(c2));
        //c2 &= 127;
    }
    switch (status & midi_command_mask)
    {
    case note_off:
        emit signalSMFNoteOff(chan, c1, c2);
        break;
    case note_on:
        emit signalSMFNoteOn(chan, c1, c2);
        break;
    case poly_aftertouch:
        emit signalSMFKeyPress(chan, c1, c2);
        break;
    case control_change:
        emit signalSMFCtlChange(chan, c1, c2);
        break;
    case program_chng:
        emit signalSMFProgram(chan, c1);
        break;
    case channel_aftertouch:
        emit signalSMFChanPress(chan, c1);
        break;
    case pitch_wheel:
        k = c1 + (c2 << 7) - 8192;
        emit signalSMFPitchBend(chan, k);
        break;
    default:
        SMFError(QString("Invalid MIDI status %1. Unhandled event").arg(status));
        break;
    }
}

void QSmf::metaEvent(quint8 b)
{
    QSmfRecTempo rec;
    QByteArray m(d->m_MsgBuff);

    switch (b)
    {
    case sequence_number:
        emit signalSMFSequenceNum(to16bit(m[0], m[1]));
        break;
    case text_event:
    case copyright_notice:
    case sequence_name:
    case instrument_name:
    case lyric:
    case marker:
    case cue_point: {
            QString s;
            if (d->m_codec == NULL)
                s = QString(m);
            else
                s = d->m_codec->toUnicode(m);
            emit signalSMFText(b, s);
        }
        break;
    case forced_channel:
        emit signalSMFforcedChannel(m[0]);
        break;
    case forced_port:
        emit signalSMFforcedPort(m[0]);
        break;
    case end_of_track:
        emit signalSMFendOfTrack();
        break;
    case set_tempo:
        d->m_CurrTempo = to32bit(0, m[0], m[1], m[2]);
        emit signalSMFTempo(d->m_CurrTempo);
        rec = d->m_TempoList.last();
        if (rec.tempo == d->m_CurrTempo)
        {
            return;
        }
        if (rec.time > d->m_CurrTime)
        {
            return;
        }
        addTempo(d->m_CurrTempo, d->m_CurrTime);
        break;
    case smpte_offset:
        emit signalSMFSmpte(m[0], m[1], m[2], m[3], m[4]);
        break;
    case time_signature:
        emit signalSMFTimeSig(m[0], m[1], m[2], m[3]);
        break;
    case key_signature:
        emit signalSMFKeySig(m[0], m[1]);
        break;
    case sequencer_specific:
        emit signalSMFSeqSpecific(m);
        break;
    default:
        emit signalSMFMetaUnregistered(b, m);
        break;
    }
    emit signalSMFMetaMisc(b, m);
}

void QSmf::sysEx()
{
    QByteArray varr(d->m_MsgBuff);
    emit signalSMFSysex(varr);
}

void QSmf::badByte(quint8 b, int p)
{
    SMFError(QString("Unexpected byte (%1) at %2").arg(b, 2, 16).arg(p));
}

quint8 QSmf::lowerByte(quint16 x)
{
    return (x & 0xff);
}

quint8 QSmf::upperByte(quint16 x)
{
    return ((x >> 8) & 0xff);
}

void QSmf::msgInit()
{
    d->m_MsgBuff.truncate(0);
}

void QSmf::msgAdd(quint8 b)
{
    int s = d->m_MsgBuff.size();
    d->m_MsgBuff.resize(s + 1);
    d->m_MsgBuff[s] = b;
}

/* public properties (accessors) */

/**
 * Gets the current time in ticks
 * @return Time in ticks
 */
long QSmf::getCurrentTime()
{
    return d->m_CurrTime;
}

/**
 * Gets the current tempo
 * @return Tempo in us per quarter
 */
long QSmf::getCurrentTempo()
{
    return d->m_CurrTempo;
}

/**
 * Gets the real time in seconds
 * @return Time in seconds
 */
long QSmf::getRealTime()
{
    return d->m_RealTime;
}

/**
 * Gets the resolution
 * @return Resolution in ticks per quarter note
 */
int QSmf::getDivision()
{
    return d->m_Division;
}

/**
 * Sets the resolution
 * @param division Resolution in ticks per quarter note
 */
void QSmf::setDivision(int division)
{
    d->m_Division = division;
}

/**
 * Gets the number of tracks
 * @return Number of tracks
 */
int QSmf::getTracks()
{
    return d->m_Tracks;
}

/**
 * Sets the number of tracks
 * @param tracks Number of tracks
 */
void QSmf::setTracks(int tracks)
{
    d->m_Tracks = tracks;
}

/**
 * Gets the SMF file format
 * @return File format (0, 1, or 2)
 */
int QSmf::getFileFormat()
{
    return d->m_fileFormat;
}

/**
 * Sets the SMF file format
 * @param fileFormat File format (0, 1, or 2)
 */
void QSmf::setFileFormat(int fileFormat)
{
    d->m_fileFormat = fileFormat;
}

/**
 * Gets the position in the SMF stream
 * @return Position offset in the stream
 */
long QSmf::getFilePos()
{
    return (long) d->m_IOStream->device()->pos();
}

/**
 * Gets the text codec used for text meta-events I/O
 * @return QTextCodec pointer
 * @since 0.2.0
 */
QTextCodec* QSmf::getTextCodec()
{
    return d->m_codec;
}

/**
 * Sets the text codec for text meta-events.
 * The engine doesn't take ownership of the codec instance.
 *
 * @param codec QTextCodec pointer
 * @since 0.2.0
 */
void QSmf::setTextCodec(QTextCodec *codec)
{
    d->m_codec = codec;
}

}
