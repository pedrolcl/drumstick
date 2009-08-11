/*
    Standard MIDI File component
    Copyright (C) 2006-2009, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef INCLUDED_SMF_H
#define INCLUDED_SMF_H

#include "aseqmmcommon.h"
#include <QObject>
#include <QDataStream>
#include <QString>
#include <QList>

BEGIN_MIDIUTIL_NAMESPACE

#define MThd 0x4d546864
#define MTrk 0x4d54726b

/* Standard MIDI Files meta event definitions */
#define meta_event          0xff
#define sequence_number     0x00
#define text_event          0x01
#define copyright_notice    0x02
#define sequence_name       0x03
#define instrument_name     0x04
#define lyric               0x05
#define marker              0x06
#define cue_point           0x07
#define forced_channel      0x20
#define forced_port         0x21
#define end_of_track        0x2f
#define set_tempo           0x51
#define smpte_offset        0x54
#define time_signature      0x58
#define key_signature       0x59
#define sequencer_specific  0x7f

/* MIDI status commands most significant bit is 1 */
#define note_off            0x80
#define note_on             0x90
#define poly_aftertouch     0xa0
#define control_change      0xb0
#define program_chng        0xc0
#define channel_aftertouch  0xd0
#define pitch_wheel         0xe0
#define system_exclusive    0xf0
#define end_of_sysex        0xf7

#define midi_command_mask   0xf0
#define midi_channel_mask   0x0f

#define major_mode          0
#define minor_mode          1

class QSmf : public QObject
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

signals:
    void signalSMFError(const QString& errorStr);
    void signalSMFHeader(int format, int ntrks, int division);
    void signalSMFNoteOn(int chan, int pitch, int vol);
    void signalSMFNoteOff(int chan, int pitch, int vol);
    void signalSMFKeyPress(int chan, int pitch, int press);
    void signalSMFCtlChange(int chan, int ctl, int value);
    void signalSMFPitchBend(int chan, int value);
    void signalSMFProgram(int chan, int patch);
    void signalSMFChanPress(int chan, int press);
    void signalSMFSysex(const QByteArray& data);
    void signalSMFVariable(const QByteArray& data);
    void signalSMFSeqSpecific(const QByteArray& data);
    void signalSMFMetaMisc(int typ, const QByteArray& data);
    void signalSMFSequenceNum(int seq);
    void signalSMFforcedChannel(int channel);
    void signalSMFforcedPort(int port);
    void signalSMFText(int typ, const QString& data);
    void signalSMFSmpte(int b0, int b1, int b2, int b3, int b4);
    void signalSMFTimeSig(int b0, int b1, int b2, int b3);
    void signalSMFKeySig(int b0, int b1);
    void signalSMFTempo(int tempo);
    void signalSMFendOfTrack();
    void signalSMFTrackStart();
    void signalSMFTrackEnd();
    void signalSMFWriteTempoTrack();
    void signalSMFWriteTrack(int track);

private:
    struct QSmfRecTempo
    {
        quint64 tempo;
        quint64 time;
    };

    QDataStream *m_IOStream;
    QByteArray m_MsgBuff;
    QList<QSmfRecTempo> m_TempoList;
    quint64 m_ToBeRead;
    quint64 m_NumBytesWritten;
    bool m_Interactive;     // file and track headers are not required
    quint64 m_CurrTime;     // current time in delta-time units
    quint64 m_RealTime;     // current time in 1/16 centisecond-time units
    double m_DblRealTime;   // as above, floating
    int m_Division;         // ticks per beat. Default = 96
    quint64 m_CurrTempo;    // microseconds per quarter note
    quint64 m_OldCurrTempo;
    quint64 m_OldRealTime;
    double m_DblOldRealtime;
    quint64 m_OldCurrTime;
    quint64 m_RevisedTime;
    quint64 m_TempoChangeTime;
    int m_Tracks;
    int m_fileFormat;
    int m_LastStatus;

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

END_MIDIUTIL_NAMESPACE

#endif
