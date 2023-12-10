/*
    Standard RIFF MIDI Component
    Copyright (C) 2006-2023, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QDebug>
#include <QIODevice>
#include <QFile>
#include <drumstick/rmid.h>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#define right Qt::right
#define left Qt::left
#define endl Qt::endl
#define hex Qt::hex
#define dec Qt::dec
#endif

/**
 * @file rmid.cpp
 * Implementation of a class managing RIFF MIDI Files input
 */

namespace drumstick { namespace File {

/**
 * @addtogroup RMID
 * @{
 *
 * Rmidi provides a mechanism to parse RIFF RMID Files, without the burden of a policy forcing to use some internal sequence representation.
 *
 * RIFF RMID is a wrapper format for MIDI data, as first specified by Microsoft, and later extended by MIDI.org (an arm of the MIDI Manufacturers Association) to permit the bundling of both MIDI files and Downloadable Sounds (DLS) files. According to Multimedia Programming Interface and Data Specifications 1.0, August 1991.: The 'RMID' format consists of a standard MIDI file enclosed in a RIFF chunk. Enclosing the MIDI file in a 'RIFF' chunk allows the file to be consistently identified; for example, an 'INFO' list can be included in the file.
 *
 * This implementation does not yet support embedded DLS data. This format is deprecated in favor of Extensible Music Files (XMF).
 *
 * This class is not related or based on the ALSA library. To parse the SMF portion of the format, the QSmf class should be used.
 *
 * @see https://www.loc.gov/preservation/digital/formats/fdd/fdd000120.shtml
 * @see http://web.archive.org/web/20110610135604/http://www.midi.org/about-midi/rp29spec(rmid).pdf
 * @}
 */

const quint32 CKID_RIFF = 0x46464952;
const quint32 CKID_LIST = 0x5453494c;
const quint32 CKID_INFO = 0x4f464e49;
const quint32 CKID_RMID = 0x44494d52;
const quint32 CKID_data = 0x61746164;
const quint32 CKID_DISP = 0x50534944;
const quint32 CKID_DLS  = 0x20534C44;

/**
 * Constructor
 * @param parent Object owner
 */
Rmidi::Rmidi(QObject * parent):
    QObject(parent)
{ }

/**
 * Destructor
 */
Rmidi::~Rmidi()
{ }

/**
 * Reads a stream from a disk file.
 * @param fileName Name of an existing file.
 */
void Rmidi::readFromFile(QString fileName)
{
    //qDebug() << Q_FUNC_INFO << fileName;
    QFile file(m_fileName = fileName);
    file.open(QIODevice::ReadOnly);
    QDataStream ds(&file);
    readFromStream(&ds);
    file.close();
}

/**
 * Reads a stream.
 * @param ds Pointer to an existing and opened input stream
 */
void Rmidi::readFromStream(QDataStream* ds)
{
    //qDebug() << Q_FUNC_INFO;
    if (ds != nullptr) {
        m_stream = ds;
        m_stream->setByteOrder(QDataStream::LittleEndian);
        read();
    }
}

QString Rmidi::toString(quint32 ckid)
{
    QByteArray data(reinterpret_cast<char *>(&ckid), sizeof(quint32));
    return QString::fromLatin1(data);
}

QByteArray Rmidi::readByteArray(int size)
{
    //qDebug() << Q_FUNC_INFO << size;
    char *buffer = new char[size];
    m_stream->readRawData(buffer, size);
    QByteArray ba(buffer);
    delete[] buffer;
    return ba;
}

void Rmidi::skip(quint32 cktype, int size)
{
    Q_UNUSED(cktype)
    //qDebug() << Q_FUNC_INFO << toString(cktype) << size;
    m_stream->skipRawData(size);
}

quint32 Rmidi::readExpectedChunk(quint32 cktype)
{
    quint32 chunkType, len = 0;
    *m_stream >> chunkType;
    if (chunkType == cktype) {
        *m_stream >> len;
        if (len % 2) len++; // alignment to even size
        /*qDebug() << Q_FUNC_INFO
                 << "Expected:" << toString(chunkType)
                 << "(" << hex << chunkType << ")"
                 << "length:" << dec << len;*/
    } /*else {
        qDebug() << Q_FUNC_INFO
                 << "Expected:" << toString(cktype)
                 << "(" << hex << cktype << ")"
                 << "got instead:" << toString(chunkType)
                 << "(" << hex << chunkType << ")";
    }*/
    return len;
}

quint32 Rmidi::readChunk(quint32& chunkType)
{
    quint32 len = 0;
    *m_stream >> chunkType;
    *m_stream >> len;
    if (len % 2) len++; // alignment to even size
    /*qDebug() << Q_FUNC_INFO
             << "chunkType:" << toString(chunkType)
             << "(" << hex << chunkType << ")"
             << "length:" << dec << len;*/
    return len;
}

quint32 Rmidi::readChunkID()
{
    quint32 chunkID;
    *m_stream >> chunkID;
    /*qDebug() << Q_FUNC_INFO
             << "chunkID:" << toString(chunkID)
             << "(" << hex << chunkID << ")";*/
    return chunkID;
}

void Rmidi::processINFO(int size)
{
    //qDebug() << Q_FUNC_INFO << size;
    quint32 chunkID = 0;
    quint32 length = 0;
    while ((size > 0) && !m_stream->atEnd()) {
        length = readChunk(chunkID);
        size -= 8;
        size -= length;
        QString cktype = toString(chunkID);
        QByteArray data = readByteArray(length);
        Q_EMIT signalRiffInfo(cktype, data);
    }
}

void Rmidi::processList(int size)
{
    //qDebug() << Q_FUNC_INFO;
    quint32 chunkID = 0;
    if (m_stream->atEnd()) return;
    chunkID = readChunkID();
    size -= 4;
    switch (chunkID) {
    case CKID_INFO:
        processINFO(size);
        break;
    default:
        skip(chunkID, size);
    }
}

void Rmidi::processRMID(int size)
{
    //qDebug() << Q_FUNC_INFO << size;
    quint32 chunkID = 0;
    int length;
    while ((size > 0) && !m_stream->atEnd()) {
        length = readChunk(chunkID);
        size -= 8;
        switch (chunkID) {
        case CKID_data:
            processData("RMID", length);
            break;
        case CKID_LIST:
            processList(length);
            break;
        case CKID_DISP:
            skip(chunkID, length);
            break;
        case CKID_RIFF:
            processRIFF(length);
            break;
        default:
            skip(chunkID, length);
        }
        size -= length;
    }
}

void Rmidi::processRIFF(int size)
{
    quint32 chunkID = readChunkID();
    quint32 length = size - 4;
    switch(chunkID) {
    case CKID_RMID:
        //qDebug() << "RMID format";
        processRMID(length);
        break;
    case CKID_DLS:
        //qDebug() << "DLS format";
        if (m_stream->device() != nullptr && m_stream->device()->pos() >= 12) {
            m_stream->device()->seek(m_stream->device()->pos() - 12);
            processData("DLS", length + 12);
        } else {
            skip(chunkID, length);
        }
        break;
    default:
        qWarning() << "Unsupported format";
        skip(chunkID, length);
    }
}

void Rmidi::processData(const QString& dataType, int size)
{
    //qDebug() << Q_FUNC_INFO << size;
    QByteArray memdata(size, '\0');
    m_stream->readRawData(memdata.data(), size);
    Q_EMIT signalRiffData(dataType, memdata);
}

void Rmidi::read()
{
    //qDebug() << Q_FUNC_INFO;
    quint32 length = readExpectedChunk(CKID_RIFF);
    if (length > 0) {
        processRIFF(length);
    }
}

}} // namespace drumstick::File
