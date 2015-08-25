/*
    Standard MIDI File creation from scratch.
    Copyright (C) 2006-2015, Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
 
    This program is free software; you can redistribute it and/or modify
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

#include "buildsmf.h"
#include "qsmf.h"
#include "cmdlineargs.h"
#include <QCoreApplication>
#include <QDebug>
#include <QTextCodec>

QSMFBuilder::QSMFBuilder() : QObject()
{
    m_engine = new QSmf(this);
    m_engine->setTextCodec(QTextCodec::codecForName("UTF-8"));
    connect(m_engine, SIGNAL(signalSMFError(const QString&)), 
            this, SLOT(errorHandler(const QString&)));
    connect(m_engine, SIGNAL(signalSMFWriteTrack(int)), 
            this, SLOT(trackHandler(int)));
}

void QSMFBuilder::errorHandler(const QString& errorStr)
{
    qWarning() << errorStr;
    exit(1);
}

void QSMFBuilder::trackHandler(int )
{
    unsigned int i;

    // Text event
    m_engine->writeMetaEvent(0, copyright_notice, 
        QString::fromUtf8("Copyright (C) 2006-2015 Pedro López-Cabanillas"));

    m_engine->writeBpmTempo(0, 100);                
    m_engine->writeTimeSignature(0, 3, 2, 36, 8);  // ts = 3/4
    m_engine->writeKeySignature(0, 2, major_mode); // D major (2 sharps)

    // system exclusive event
    static unsigned char gsreset[] = { 0xf0, 0x41, 0x10, 0x42, 0x12,
                                       0x40, 0x00, 0x7f, 0x00, 0x41, 0xf7 };
    m_engine->writeMidiEvent(0, system_exclusive, sizeof(gsreset), (char *) &gsreset);

    // some note events
    static char notes[] = { 60, 62, 64, 65, 67, 69, 71, 72 };
    for(i = 0; i < sizeof(notes); ++i)
    {
        m_engine->writeMidiEvent(0,  note_on,  0, notes[i], 120);
        m_engine->writeMidiEvent(60, note_off, 0, notes[i], 0);
    }

    // final event
    m_engine->writeMetaEvent(0, end_of_track); 
}

void QSMFBuilder::run(QString fileName)
{
    m_engine->setDivision(120);
    m_engine->setFileFormat(0);
    m_engine->setTracks(1);
    m_engine->writeToFile(fileName);
}

int main(int argc, char **argv)
{
    QSMFBuilder builder;
    CmdLineArgs args;
    args.setUsage("file");
    args.addRequiredArgument("file", "Output SMF name");
    args.parse(argc, argv);
    QVariant file = args.getArgument("file");
    if (!file.isNull())
        builder.run(file.toString());
    return 0;
}

