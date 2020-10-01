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

#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <drumstick/pianopalette.h>
#include "pianokey.h"

/**
 * @file pianokey.cpp
 * Implementation of the PianoKey class
 */

namespace drumstick { namespace widgets {

const PianoPalette PianoKey::keyPalette(PAL_KEYS);

PianoKey::PianoKey(const QRectF &rect, const bool black, const int note)
    : QGraphicsRectItem(rect),
    m_pressed(false),
    m_note(note),
    m_black(black)
{
    m_brush = keyPalette.getColor(black ? 1 : 0);
    setAcceptedMouseButtons(Qt::NoButton);
}

void PianoKey::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    static const QPen blackPen(Qt::black, 1);
    static const QPen grayPen(QBrush(Qt::gray), 1, Qt::SolidLine,  Qt::RoundCap, Qt::RoundJoin);
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

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
    painter->drawRoundedRect(rect(), 15, 15, Qt::RelativeSize);
    if (m_black) {
        painter->drawPixmap(rect(), getPixmap(), pixmapRect());
    } else {
        QPointF points[3] = {
             QPointF(rect().left()+1.5, rect().bottom()-1),
             QPointF(rect().right()-1, rect().bottom()-1),
             QPointF(rect().right()-1, rect().top()+1),
        };
        painter->setPen(grayPen);
        painter->drawPolyline(points, 3);
    }
}

void PianoKey::setPressed(bool p)
{
    if (p != m_pressed) {
        m_pressed = p;
        update();
    }
}

QPixmap& PianoKey::getPixmap() const
{
    static QPixmap pixmap(QStringLiteral(":/vpiano/blkey.png"));
    return pixmap;
}

QRectF PianoKey::pixmapRect() const
{
    return getPixmap().rect();
}

void PianoKey::resetBrush()
{
    m_brush = keyPalette.getColor(m_black ? 1 : 0);
}

}} // namespace drumstick::widgets
