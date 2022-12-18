/*
    Standard RIFF MIDI Component
    Copyright (C) 2006-2022, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef RMIDI_H
#define RMIDI_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QDataStream>
#include "macros.h"

/**
 * @file rmid.h
 * RIFF MIDI Files Input
 */

namespace drumstick { namespace File {

/**
 * @addtogroup RMID RIFF MIDI File Parser (Input)
 * @{
 */

/**
 * RIFF MIDI file format (input only)
 *
 * This class is used to parse RIFF MIDI Files
 * @since 2.4.0
 */
class DRUMSTICK_EXPORT Rmidi : public QObject
{
    Q_OBJECT

public:
    explicit Rmidi(QObject * parent = nullptr);
    virtual ~Rmidi();

    void readFromFile(QString fileName);
    void readFromStream(QDataStream* ds);

signals:
    /**
     * @brief signalRMidInfo is emitted for each RIFF INFO element
     * @param infoType Type of data (chunk ID) as defined in the spec (Source: www.midi.org rp29spec.pdf).
     *
     * * IARL Archival Location. Indicates where the subject of the file is archived.
     * * IART Artist. Lists the artist (author) of the original subject of the file. For example: Les Getalong.
     * * ICMS Commissioned. Lists the name of the person or organization that commissioned the subject of the file. For example: Acme Consolidated GameWorks.
     * * ICMT Comments. Provides general comments about the file or the subject of the file. If the comment is several sentences long, end each sentence with a period. Do not include newline characters.
     * * ICOP Copyright. Records the copyright information for the file. For example: Copyright Acme Consolidated GameWorks 1991. If there are multiple copyrights, separate them by a semicolon followed by a space.
     * * ICRD Creation date. Specifies the date the subject of the file was created. List dates in year-month-day format, padding one-digit months and days with a zero on the left. For example: 1553-05-03 for May 3, 1553. The year should always be given using four digits.
     * * IENG Engineer. Stores the name of the engineer who worked on the file. If there are multiple engineers, separate the names by a semicolon and a blank. For example: Smith, John; Adams, Joe.
     * * IGNR Genre. Describes the original work, such as jazz, classical, rock, techno, rave, neo british pop grunge metal, etc.
     * * IKEY Keywords. Provides a list of keywords that refer to the file or subject of the file. Separate multiple keywords with a semicolon and a blank. For example: FX; visitation; space alien.
     * * IMED Medium. Describes the original subject of the file, such as record, CD and so forth.
     * * INAM Name. Stores the title of the subject of the file, such as Seattle From Above.
     * * IPRD Product. Specifies the name of the title the file was originally intended for, such as Galactic Ambassadors V.
     * * ISBJ Subject. Describes the contents of the file, such as Music of the Gnu Whirled Order.
     * * ISFT Software. Identifies the name of the software package used to create the file, such as Crash Compactor, Acme Consolidated Sonic Booms.
     * * ISRC Source. Identifies the name of the person or organization who supplied the original subject of the file. For example: Acme Hysterical Media Archives.
     * * ISRF Source Form. Identifies the original form of the material that was digitized, such as record, sampling CD, TV sound track and so forth. This is not necessarily the same as IMED.
     * * ITCH Technician. Identifies the technician who sampled the subject file. For example: Smith, John.
     *
     * @param info Text data
     */
    void signalRiffInfo(const QString& infoType, const QByteArray& info);

    /**
     * @brief signalRiffData is emitted for each RMID data element
     *
     * The handler of this event should use the method QSmf::readFromStream() to
     * parse the contents of the SMF data element.
     *
     * @param dataType may be "RMID" (SMF) or "DLS"
     * @param data binary payload, in RMID files is either a Standard MIDI File or a DLS structure
     */
    void signalRiffData(const QString& dataType, const QByteArray& data);

private:
    void read();
    void processINFO(int size);
    void processList(int size);
    void processRMID(int size);
    void processRIFF(int size);
    void processData(const QString& dataType, int size);
    void skip(quint32 cktype, int size);
    quint32 readExpectedChunk(quint32 cktype);
    quint32 readChunk(quint32& cktype);
    quint32 readChunkID();
    QByteArray readByteArray(int size);
    QString toString(quint32 ckid);

private:
    QDataStream *m_stream;
    QString m_fileName;
};

/** @} */

}} // namespace drumstick::File

#endif // RMIDI_H
