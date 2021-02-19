/*
    Virtual Piano Widget for Qt5
    Copyright (C) 2008-2021, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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
#include <drumstick/pianopalette.h>

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
    m_black(black),
    m_usePixmap(true)
{
    m_brush = keyPalette.getColor(black ? 1 : 0);
    setAcceptedMouseButtons(Qt::NoButton);
}

void PianoKey::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    static const QPen blackPen(Qt::black, 1);
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
    painter->drawRoundedRect(rect(), 20, 15, Qt::RelativeSize);
    if (m_usePixmap) {
        painter->drawPixmap(rect(), getPixmap(), pixmapRect());
    }
}

void PianoKey::setPressed(bool p)
{
    if (p != m_pressed) {
        m_pressed = p;
        update();
    }
}

const QPixmap& PianoKey::getPixmap() const
{
    static QPixmap blpixmap(QStringLiteral(":/vpiano/blkey.png"));
    static QPixmap whpixmap(QStringLiteral(":/vpiano/whkey.png"));
    if (m_pixmap.isNull()) {
        return m_black ? blpixmap : whpixmap;
    } else {
        return m_pixmap;
    }
}

QRectF PianoKey::pixmapRect() const
{
    return getPixmap().rect();
}

void PianoKey::resetBrush()
{
    m_brush = keyPalette.getColor(m_black ? 1 : 0);
}

void PianoKey::setPixmap(const QPixmap &p)
{
    m_pixmap = p;
}

bool PianoKey::getUsePixmap() const
{
    return m_usePixmap;
}

void PianoKey::setUsePixmap(bool usePixmap)
{
    m_usePixmap = usePixmap;
}

} // namespace widgets
} // namespace drumstick
