/*
    MIDI Virtual Piano Keyboard
    Copyright (C) 2008-2020, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PIANOPALETTE_H
#define PIANOPALETTE_H

#include <QColor>
#include <QList>
#include "macros.h"

namespace drumstick { namespace widgets {

enum PalettePolicy {
    PAL_SINGLE = 0,
    PAL_DOUBLE = 1,
    PAL_CHANNELS = 2,
    PAL_SCALE = 3
};

class DRUMSTICK_EXPORT PianoPalette
{
public:
    PianoPalette(int maxcolors, int id);
    virtual ~PianoPalette() {}

    int paletteId() { return m_paletteId; }

    QString paletteName() const { return m_paletteName; }
    void setPaletteName(const QString name);

    QString paletteText() const { return m_paletteText; }
    void setPaletteText(const QString text);

    void setColor(int n, QString s, QColor c);
    void setColor(int n, QColor c);
    void setColorName(int n, QString s);
    QColor getColor(int i);
    QString getColorName(int i);

    int getNumColors();
    void saveColors();
    void loadColors();

protected:
    int m_paletteId;
    QList<QColor> m_colors;
    QList<QString> m_names;
    QString m_paletteName;
    QString m_paletteText;
};

}} // namespace drumstick::widgets

#endif // PIANOPALETTE_H
