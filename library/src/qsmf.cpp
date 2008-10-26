/*
    Standard MIDI File component 
    Copyright (C) 2006-2008, Pedro Lopez-Cabanillas <plcl@users.sf.net>
 
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

#include <limits>
#include <QObject>
#include <QFile>
#include "qsmf.h"

namespace MIDI
{
namespace Utils
{

QSmf::QSmf(QObject * parent) :
    QObject(parent)
{
    m_Interactive = false;
    m_CurrTime = 0;
    m_RealTime = 0;
    m_DblRealTime = 0;
    m_DblOldRealtime = 0;
    m_Division = 96;
    m_CurrTempo = 500000;
    m_OldCurrTempo = 500000;
    m_OldRealTime = 0;
    m_OldCurrTime = 0;
    m_RevisedTime = 0;
    m_TempoChangeTime = 0;
}

QSmf::~QSmf()
{
    m_TempoList.clear();
}

bool QSmf::endOfSmf()
{
    return m_IOStream->atEnd();
}

quint8 QSmf::getByte()
{
    quint8 b = 0;
    if (!m_IOStream->atEnd())
    {
        *m_IOStream >> b;
        m_ToBeRead--;
    }
    return b;
}

void QSmf::putByte(quint8 value)
{
    *m_IOStream << value;
    m_NumBytesWritten++;
}

void QSmf::addTempo(quint64 tempo, quint64 time)
{
    QSmfRecTempo tempoRec;
    tempoRec.tempo = tempo;
    tempoRec.time = time;
    m_TempoList.append(tempoRec);
}

void QSmf::readHeader()
{
    m_CurrTime = 0;
    m_RealTime = 0;
    m_Division = 96;
    m_CurrTempo = 500000;
    m_OldCurrTempo = 500000;
    addTempo(m_CurrTempo, 0);
    if (m_Interactive)
    {
        m_fileFormat= 0;
        m_Tracks = 1;
        m_Division = 96;
    }
    else
    {
        readExpected("MThd");
        m_ToBeRead = read32bit();
        m_fileFormat = read16bit();
        m_Tracks = read16bit();
        m_Division = read16bit();
    }
    emit signalSMFHeader(m_fileFormat, m_Tracks, m_Division);

    /* flush any extra stuff, in case the length of header is not */
    while ((m_ToBeRead > 0) && !endOfSmf())
    {
        getByte();
    }
}

/* read a track chunk */
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
    if (m_Interactive)
    {
        m_ToBeRead = std::numeric_limits<unsigned long long>::max();
    }
    else
    {
        readExpected("MTrk");
        m_ToBeRead = read32bit();
    }
    m_CurrTime = 0;
    m_RealTime = 0;
    m_DblRealTime = 0;
    m_DblOldRealtime = 0;
    m_OldCurrTime = 0;
    m_OldRealTime = 0;
    m_CurrTempo = findTempo();

    emit signalSMFTrackStart();

    while (!endOfSmf() && (m_Interactive || m_ToBeRead > 0))
    {
        if (m_Interactive)
        {
            m_CurrTime++;
        }
        else
        {
            delta_ticks = readVarLen();
            m_RevisedTime = m_CurrTime;
            m_CurrTime += delta_ticks;
            while (m_RevisedTime < m_CurrTime)
            {
                save_time = m_RevisedTime;
                save_tempo = m_CurrTempo;
                m_CurrTempo = findTempo();
                if (m_CurrTempo != m_OldCurrTempo)
                {
                    m_OldCurrTempo = m_CurrTempo;
                    m_OldRealTime = m_RealTime;
                    if (m_RevisedTime != m_TempoChangeTime)
                    {
                        m_DblOldRealtime = m_DblRealTime;
                        m_OldCurrTime = save_time;
                    }
                    delta_secs = ticksToSecs(m_RevisedTime - m_OldCurrTime,
                                             m_Division, save_tempo);
                    m_DblRealTime = m_DblOldRealtime + delta_secs * 1600.0;
                    m_RealTime = static_cast<quint64>(0.5 + m_DblRealTime);
                    if (m_RevisedTime == m_TempoChangeTime)
                    {
                        m_OldCurrTime = m_RevisedTime;
                        m_DblOldRealtime = m_DblRealTime;
                    }
                }
                else
                {
                    delta_secs = ticksToSecs(m_RevisedTime - m_OldCurrTime,
                                             m_Division, m_CurrTempo);
                    m_DblRealTime = m_DblOldRealtime + delta_secs * 1600.0;
                    m_RealTime = static_cast<quint64>(0.5 + m_DblRealTime);
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
            lookfor = m_ToBeRead - lookfor;
            msgInit();
            while (m_ToBeRead > lookfor)
            {
                msgAdd(getByte());
            }
            metaEvent(type);
            break;
        case system_exclusive:
            lookfor = readVarLen();
            lookfor = m_ToBeRead - lookfor;
            msgInit();
            msgAdd(system_exclusive);
            while (m_ToBeRead > lookfor)
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
            lookfor = m_ToBeRead - lookfor;
            if (!sysexcontinue)
            {
                msgInit();
            }
            while (m_ToBeRead > lookfor)
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
            badByte(c, m_IOStream->device()->pos() - 1);
            break;
        }
    }
    emit signalSMFTrackEnd();
}

void QSmf::SMFRead()
{
    int i;
    readHeader();
    for ( i = m_Tracks; (i > 0) && !endOfSmf(); i--)
    {
        readTrack();
    }
}

/**
 * Every MIDI file starts with a header
 * In format 1 files, the first track is a tempo map
 * The rest of the file is a series of tracks 
 */
void QSmf::SMFWrite()
{
    int i;
    m_LastStatus = 0;
    writeHeaderChunk(m_fileFormat, m_Tracks, m_Division);
    m_LastStatus = 0;
    if (m_fileFormat == 1)
    {
        emit signalSMFWriteTempoTrack();
    }
    for (i = 0; i < m_Tracks; ++i)
    {
        writeTrackChunk(i);
    }
}

void QSmf::readFromStream(QDataStream *stream)
{
    m_IOStream = stream;
    SMFRead();
}

void QSmf::readFromFile(const QString& fileName)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QDataStream ds(&file);
    readFromStream(&ds);
    file.close();
}

void QSmf::writeToStream(QDataStream *stream)
{
    m_IOStream = stream;
    SMFWrite();
}

void QSmf::writeToFile(const QString& fileName)
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    QDataStream ds(&file);
    writeToStream(&ds);
    file.close();
}

void QSmf::writeHeaderChunk(int format, int ntracks, int division)
{
    write32bit(MThd);
    write32bit(6);
    write16bit(format);
    write16bit(ntracks);
    write16bit(division);
}

void QSmf::writeTrackChunk(int track)
{
    quint32 trkhdr;
    quint32 trklength;
    qint64 offset;
    qint64 place_marker;

    m_LastStatus = 0;
    trkhdr = MTrk;
    trklength = 0;
    offset = m_IOStream->device()->pos();
    write32bit(trkhdr);
    write32bit(trklength);
    m_NumBytesWritten = 0;

    emit signalSMFWriteTrack(track);

    place_marker = m_IOStream->device()->pos();
    m_IOStream->device()->seek(offset);
    trklength = m_NumBytesWritten;
    write32bit(trkhdr);
    write32bit(trklength);
    m_IOStream->device()->seek(place_marker);
}

void QSmf::writeMetaEvent(long deltaTime, int type, const QByteArray& data)
{
    int i;
    writeVarLen(deltaTime);
    m_LastStatus = meta_event;
    putByte(m_LastStatus);
    putByte(type);
    writeVarLen(data.size());
    for (i = 0; i < data.size(); ++i)
    {
        putByte(data[i]);
    }
}

void QSmf::writeMetaEvent(long deltaTime, int type, const QString& data)
{
    int i;
    writeVarLen(deltaTime);
    putByte(m_LastStatus = meta_event);
    putByte(type);
    writeVarLen(data.length());
    const char *asciichars = data.toLocal8Bit().data();
    for (i = 0; i < data.length(); ++i)
    {
        putByte(asciichars[i]);
    }
}

void QSmf::writeMetaEvent(long deltaTime, int type)
{
    writeVarLen(deltaTime);
    putByte(m_LastStatus = meta_event);
    putByte(type);
    putByte(0);
}

void QSmf::writeMidiEvent(long deltaTime, int type, int chan,
                          const QByteArray& data)
{
    int i, j, size;
    quint8 c;
    writeVarLen(deltaTime);
    if ((type == system_exclusive) || (type == end_of_sysex))
    {
        c = type;
        m_LastStatus = 0;
    }
    else
    {
        if (chan > 15)
        {
            SMFError("error: MIDI channel greater than 16");
        }
        c = type | chan;
    }
    if (m_LastStatus != c)
    {
        m_LastStatus = c;
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
    if (m_LastStatus != c)
    {
        m_LastStatus = c;
        putByte(c);
    }
    putByte(b1);
}

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
    if (m_LastStatus != c)
    {
        m_LastStatus = c;
        putByte(c);
    }
    putByte(b1);
    putByte(b2);
}

void QSmf::writeMidiEvent(long deltaTime, int type, long len, char* data)
{
    unsigned int i, j, size;
    quint8 c;
    writeVarLen(deltaTime);
    if ((type != system_exclusive) && (type != end_of_sysex))
    {
        SMFError("error: type should be system exclusive");
    }
    m_LastStatus = 0;
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

void QSmf::writeSequenceNumber(long deltaTime, int seqnum)
{
    writeVarLen(deltaTime);
    m_LastStatus = meta_event;
    putByte(m_LastStatus);
    putByte(sequence_number);
    putByte(2);
    putByte((seqnum >> 8) & 0xff);
    putByte(seqnum & 0xff);
}

/* tempo expressed in microseconds per quarter note */
void QSmf::writeTempo(long deltaTime, long tempo)
{
    writeVarLen(deltaTime);
    putByte(m_LastStatus = meta_event);
    putByte(set_tempo);
    putByte(3);
    putByte((tempo >> 16) & 0xff);
    putByte((tempo >> 8) & 0xff);
    putByte(tempo & 0xff);
}

/* tempo expressed in quarter notes per minute */
void QSmf::writeBpmTempo(long deltaTime, int tempo)
{
    long us_tempo = 60000000l / tempo;
    writeVarLen(deltaTime);
    putByte(m_LastStatus = meta_event);
    putByte(set_tempo);
    putByte(3);
    putByte((us_tempo >> 16) & 0xff);
    putByte((us_tempo >> 8) & 0xff);
    putByte(us_tempo & 0xff);
}

void QSmf::writeTimeSignature(long deltaTime, int num, int den, int cc, int bb)
{
    writeVarLen(deltaTime);
    putByte(m_LastStatus = meta_event);
    putByte(time_signature);
    putByte(4);
    putByte(num & 0xff);
    putByte(den & 0xff);
    putByte(cc & 0xff);
    putByte(bb & 0xff);
}

void QSmf::writeKeySignature(long deltaTime, int tone, int mode)
{
    writeVarLen(deltaTime);
    putByte(m_LastStatus = meta_event);
    putByte(key_signature);
    putByte(2);
    putByte((char)tone);
    putByte(mode & 0x01);
}

/* Write multi-length bytes to MIDI format files */
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
            SMFError(QString("Invalid (%1) SMF format at %2").arg(b, 0, 16).arg(m_IOStream->device()->pos()));
            break;
        }
    }
}

quint64 QSmf::findTempo()
{
    quint64 result, old_tempo, new_tempo;
    QSmfRecTempo rec = m_TempoList.last();
    old_tempo = m_CurrTempo;
    new_tempo = m_CurrTempo;
    QList<QSmfRecTempo>::Iterator it;
    for( it = m_TempoList.begin(); it != m_TempoList.end(); ++it )
    {
        rec = (*it);
        if (rec.time <= m_CurrTime)
        {
            old_tempo = rec.tempo;
        }
        new_tempo = rec.tempo;
        if (rec.time > m_RevisedTime)
        {
            break;
        }
    }
    if ((rec.time <= m_RevisedTime) || (rec.time > m_CurrTime))
    {
        m_RevisedTime = m_CurrTime;
        result = old_tempo;
    }
    else
    {
        m_RevisedTime = rec.time;
        m_TempoChangeTime = m_RevisedTime;
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
        SMFError(QString("ChannelMessage: bad c1 = %1").arg(c1));
    }
    if (c2 > 127)
    {
        c2 = 127;
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
        qWarning("Unknown MIDI status, unhandled event");
        break;
    }
}

void QSmf::metaEvent(quint8 b)
{
    QSmfRecTempo rec;
    QByteArray m(m_MsgBuff);

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
    case cue_point:
        emit signalSMFText(b, QString(m));
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
        m_CurrTempo = to32bit(0, m[0], m[1], m[2]);
        emit signalSMFTempo(m_CurrTempo);
        rec = m_TempoList.last();
        if (rec.tempo == m_CurrTempo)
        {
            return;
        }
        if (rec.time > m_CurrTime)
        {
            return;
        }
        addTempo(m_CurrTempo, m_CurrTime);
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
        emit signalSMFMetaMisc(b, m);
        break;
    }
}

void QSmf::sysEx()
{
    QByteArray varr(m_MsgBuff);
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
    m_MsgBuff.truncate(0);
}

void QSmf::msgAdd(quint8 b)
{
    int s = m_MsgBuff.size();
    m_MsgBuff.resize(s + 1);
    m_MsgBuff[s] = b;
}

/* public properties (accessors) */

long QSmf::getCurrentTime()
{
    return m_CurrTime;
}

long QSmf::getCurrentTempo()
{
    return m_CurrTempo;
}

long QSmf::getRealTime()
{
    return m_RealTime;
}

int QSmf::getDivision()
{
    return m_Division;
}

void QSmf::setDivision(int division)
{
    m_Division = division;
}

int QSmf::getTracks()
{
    return m_Tracks;
}

void QSmf::setTracks(int tracks)
{
    m_Tracks = tracks;
}

int QSmf::getFileFormat()
{
    return m_fileFormat;
}

void QSmf::setFileFormat(int fileFormat)
{
    m_fileFormat = fileFormat;
}

long QSmf::getFilePos()
{
    return (long) m_IOStream->device()->pos();
}

}
}
