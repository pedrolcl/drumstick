/*
    Virtual Piano Widget for Qt
    Copyright (C) 2008-2024, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

/**
 * @file keylabel.h
 * Declaration of the KeyLabel class
 */

namespace drumstick {
namespace widgets {

class KeyLabel : public QGraphicsTextItem
{
public:
    explicit KeyLabel(QGraphicsItem *parent = nullptr);
    virtual ~KeyLabel() = default;
    void setPlainText(const QString& text);
    void adjust();
    void setOrientation(LabelOrientation  ori);
    void restoreColor();
    void setHtml(const QString& text);

private:
    LabelOrientation m_orientation = HorizontalOrientation;
    void calculateRotation();

    QColor m_savedColor;
};

}} // namespace drumstick::widgets

#endif // KEYLABEL_H
