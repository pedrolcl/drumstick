/*
    Virtual Piano Widget for Qt
    Copyright (C) 2008-2023, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include "keylabel.h"
#include "pianokey.h"
#include <QFont>

/**
 * @file keylabel.cpp
 * Implementation of the KeyLabel class
 */

namespace drumstick { namespace widgets {

KeyLabel::KeyLabel(QGraphicsItem *parent) : QGraphicsTextItem(parent)
{
    setAcceptedMouseButtons(Qt::NoButton);
}

void KeyLabel::adjust()
{
    qreal ax, ay;
    QRectF kr, br;
    PianoKey* key = static_cast<PianoKey*>(parentItem());
    kr = key->boundingRect();
    br = boundingRect();
    ax = kr.x();
    ay = kr.height() - 5;
    if (key->isBlack()) {
        ay -= 70;
    }
    if (rotation() == 0) {
        ax += (kr.width() - br.width()) / 2;
        ay -= br.height();
    } else {
        ax += (kr.width() - br.height()) / 2;
    }
    setPos(ax, ay);
    m_savedColor = defaultTextColor();
}

void KeyLabel::setOrientation(LabelOrientation ori)
{
    if (m_orientation != ori) {
        m_orientation = ori;
        switch(m_orientation) {
        case VerticalOrientation:
            setRotation(270);
            break;
        case HorizontalOrientation:
            setRotation(0);
            break;
        case AutomaticOrientation:
        default:
            calculateRotation();
            break;
        }
    }
}

void KeyLabel::restoreColor()
{
    if (m_savedColor.isValid()) {
        setDefaultTextColor(m_savedColor);
    }
}

void drumstick::widgets::KeyLabel::calculateRotation()
{
    PianoKey* key = static_cast<PianoKey*>(parentItem());
    QRectF kr, br;
    kr = key->boundingRect();
    br = boundingRect();
    if (br.width() > kr.width()) {
        setRotation(270);
    } else {
        setRotation(0);
    }
}

void KeyLabel::setPlainText(const QString &text)
{
    QGraphicsTextItem::setPlainText(text);
    adjustSize();
    if (m_orientation == AutomaticOrientation) {
        calculateRotation();
    }
}

void KeyLabel::setHtml(const QString &text)
{
    QGraphicsTextItem::setHtml(text);
    adjustSize();
    if (m_orientation == AutomaticOrientation) {
        calculateRotation();
    }
}

} // namespace widgets
} // namespace drumstick
