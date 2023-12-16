/*
    Drumstick MIDI realtime input-output
    Copyright (C) 2009-2023 Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MIDIPARSER_H
#define MIDIPARSER_H

#include <QObject>
#include <drumstick/rtmidiinput.h>

namespace drumstick {
namespace rt {

class MIDIParser : public QObject
{
    Q_OBJECT
public:
    explicit MIDIParser(MIDIInput *in = nullptr, QObject *parent = nullptr);
    virtual ~MIDIParser();
    void setMIDIThruDevice(MIDIOutput* device);

public Q_SLOTS:
    void parse(unsigned char byte);
    void parse(QByteArray bytes);

private:
    class MIDIParserPrivate;
    MIDIParserPrivate *d;
};

}}

#endif // MIDIPARSER_H
