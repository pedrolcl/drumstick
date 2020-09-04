/*
    Virtual Piano Widget for Qt5
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

#ifndef KEYLABEL_H
#define KEYLABEL_H

#include <QGraphicsTextItem>
#include <drumstick/pianokeybd.h>

namespace drumstick {
namespace widgets {

class KeyLabel : public QGraphicsTextItem //QGraphicsSimpleTextItem
{
public:
    KeyLabel(QGraphicsItem *parent = 0);
    virtual ~KeyLabel() {}
    void setPlainText(const QString& text);
    void adjust();
    void setOrientation(PianoKeybd::LabelOrientation  ori);
    void restoreColor();

private:
    PianoKeybd::LabelOrientation m_orientation = PianoKeybd::HorizontalOrientation;
    void calculateRotation();

    QColor m_savedColor;
};

}} // namespace drumstick::widgets

#endif // KEYLABEL_H
