/*
    Virtual Piano Widget for Qt4 
    Copyright (C) 2008, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include "pianokey.h"
#include <QApplication>
#include <QPainter>
#include <QPalette>

PianoKey::PianoKey(const QRectF &rect, const QBrush &brush, const int note) 
    : QGraphicsRectItem(rect),
    m_pressed(false),
    m_brush(brush), 
    m_note(note) 
{
    setAcceptedMouseButtons(Qt::NoButton);
}

void PianoKey::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    static const QPen blackPen(QPen(Qt::black, 1));
    if (m_pressed) {
        if (m_selectedBrush.style() != Qt::NoBrush) {
            painter->setBrush(m_selectedBrush);
        } else {
            painter->setBrush(QApplication::palette().highlight());
        }
    } else {
        painter->setBrush(m_brush);
    }
    painter->setPen(blackPen);
    painter->drawRoundRect(rect(), 15, 15);
}

void PianoKey::setPressed(bool p)
{
    if (p != m_pressed) {
        m_pressed = p;
        update();
    }
}
