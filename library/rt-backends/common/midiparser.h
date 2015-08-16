/*
    Drumstick MIDI realtime input-output
    Copyright (C) 2009-2015 Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef MIDIPARSER_H
#define MIDIPARSER_H

#include <QObject>
#include "rtmidiinput.h"

namespace drumstick {
namespace rt {

class MIDIParser : public QObject
{
    Q_OBJECT
public:
    explicit MIDIParser(MIDIInput *in = 0, QObject *parent = 0);
    virtual ~MIDIParser();
    void setMIDIThruDevice(MIDIOutput* device);

public slots:
    void parse(unsigned char byte);
    void parse(QByteArray bytes);

private:
    class MIDIParserPrivate;
    MIDIParserPrivate *d;
};

}}

#endif // MIDIPARSER_H
