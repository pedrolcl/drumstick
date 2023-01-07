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

#ifndef PIANOKEY_H_
#define PIANOKEY_H_

#include <QGraphicsRectItem>
#include <QBrush>

/**
 * @file pianokey.h
 * Declaration of the PianoKey class
 */

namespace drumstick { namespace widgets {

    class PianoPalette;

    class PianoKey : public QGraphicsRectItem
    {
    public:
        explicit PianoKey(QGraphicsItem * parent = nullptr ): QGraphicsRectItem(parent),
            m_pressed(false),
            m_note(0),
            m_black(false),
            m_usePixmap(true)
        { }
        PianoKey(const QRectF &rect, const bool black, const int note);
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
        int getNote() const { return m_note; }
        void setBrush(const QBrush& b) { m_brush = b; }
        void setPressedBrush(const QBrush& b) { m_selectedBrush = b; }
        void resetBrush();
        bool isPressed() const { return m_pressed; }
        void setPressed(bool p);
        int getDegree() const { return m_note % 12; }
        int getType() const { return (m_black ? 1 : 0); }
        bool isBlack() const { return m_black; }
        const QPixmap& getPixmap() const;
        void setPixmap(const QPixmap& p);
        QRectF pixmapRect() const;
        bool getUsePixmap() const;
        void setUsePixmap(bool usePixmap);
        void paintPixmap(QPixmap &pixmap, const QColor& color) const;

        static const PianoPalette keyPalette;

    private:
        bool m_pressed;
        QBrush m_selectedBrush;
        QBrush m_brush;
        int m_note;
        bool m_black;
        QPixmap m_pixmap;
        bool m_usePixmap;
    };

}} // namespace drumstick::widgets

#endif /*PIANOKEY_H_*/
