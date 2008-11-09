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

#include "pianoscene.h"
#include "pianokey.h"

#include <QApplication>
#include <QPalette>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>

#define KEYWIDTH  18
#define KEYHEIGHT 72

PianoScene::PianoScene ( const int baseOctave, 
                         const int numOctaves, 
                         const QColor& keyPressedColor,
                         QObject * parent )
    : QGraphicsScene( QRectF(0, 0, KEYWIDTH * numOctaves * 7, KEYHEIGHT), parent ),
    m_baseOctave( baseOctave ),
    m_numOctaves( numOctaves ),
    m_minNote( 0 ),
    m_maxNote( 127 ),
    m_keyPressedColor( keyPressedColor ),
    m_mousePressed( false ),
    m_handler( NULL )
{
    QBrush blackBrush(Qt::black);
    QBrush whiteBrush(Qt::white);
    QBrush hilightBrush(m_keyPressedColor.isValid() ? m_keyPressedColor : QApplication::palette().highlight());
    int i, numkeys = m_numOctaves * 12;
    for(i = 0; i < numkeys; ++i)
    {
        PianoKey* key = NULL;
        int octave = i / 12 * 7;
        int j = i % 12;
        if (j >= 5) j++;
        if ((j % 2) == 0) {
            float x = (octave + j / 2) * KEYWIDTH;
            key = new PianoKey( QRectF(x, 0, KEYWIDTH, KEYHEIGHT), whiteBrush, i );
        } else {
            float x = (octave + j / 2) * KEYWIDTH + KEYWIDTH * 6/10 + 1;
            key = new PianoKey( QRectF( x, 0, KEYWIDTH * 8/10 - 1, KEYHEIGHT * 6/10 ), blackBrush, i );
            key->setZValue( 1 );
        }
        if (m_keyPressedColor.isValid())
            key->setPressedBrush(hilightBrush);
        m_keys.insert(i, key);
        addItem( key );
    }
    hideOrShowKeys();
}

QSize PianoScene::sizeHint() const
{
    return QSize(KEYWIDTH * m_numOctaves * 7, KEYHEIGHT);
}

void PianoScene::showKeyOn( PianoKey* key )
{
    key->setPressed(true);
}

void PianoScene::showKeyOff( PianoKey* key )
{
    key->setPressed(false);
}

void PianoScene::showNoteOn( const int note )
{
    int n = note - m_baseOctave*12;
    if ((note >= m_minNote) && (note <= m_maxNote) &&
        (n >= 0) && (n < m_keys.size()))
        showKeyOn(m_keys[n]);
}

void PianoScene::showNoteOff( const int note )
{
    int n = note - m_baseOctave*12;
    if ((note >= m_minNote) && (note <= m_maxNote) &&
        (n >= 0) && (n < m_keys.size()))
        showKeyOff(m_keys[n]);
}

void PianoScene::keyOn( PianoKey* key )
{
    int n = m_baseOctave*12 + key->getNote();
    if ((n >= m_minNote) && (n <= m_maxNote)) {
        if (m_handler != NULL) {
            m_handler->noteOn(n);
        } else {
            emit noteOn(n);
        }
        showKeyOn(key);
    }
}

void PianoScene::keyOff( PianoKey* key )
{
    int n = m_baseOctave*12 + key->getNote();
    if ((n >= m_minNote) && (n <= m_maxNote)) {
        if (m_handler != NULL) {
            m_handler->noteOff(n);
        } else {
            emit noteOff(n);
        }
        showKeyOff(key);
    }
}

PianoKey* PianoScene::getKeyForPos( const QPointF& p ) const
{
    QGraphicsItem *itm = itemAt(p);
    if (itm != NULL) {
        PianoKey* key = dynamic_cast<PianoKey*>(itm);
        return key;
    }
    return NULL;
}

void PianoScene::mouseMoveEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
    if (m_mousePressed) {
        PianoKey* key = getKeyForPos(mouseEvent->scenePos());
        PianoKey* lastkey = getKeyForPos(mouseEvent->lastScenePos());
        if ((lastkey != NULL) && (lastkey != key) && lastkey->isPressed()) {
            keyOff(lastkey);
        }
        if ((key != NULL) && !key->isPressed()) { 
            keyOn(key);
        }
        mouseEvent->accept();
        return;
    }
    mouseEvent->ignore();
}

void PianoScene::mousePressEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
    PianoKey* key = getKeyForPos(mouseEvent->scenePos());
    if (key != NULL) {
        keyOn(key);
        m_mousePressed = true;
        mouseEvent->accept();
        return;
    }
    mouseEvent->ignore();
}

void PianoScene::mouseReleaseEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
    m_mousePressed = false; 
    PianoKey* key = getKeyForPos(mouseEvent->scenePos());
    if (key != NULL) {
        keyOff(key);
        mouseEvent->accept();
        return;
    }
    mouseEvent->ignore();
}

PianoKey* PianoScene::getPianoKey( const int key ) const
{
    if (m_keybdMap != NULL) {
        KeyboardMap::ConstIterator it = m_keybdMap->find(key);
        if ((it != m_keybdMap->end()) && (it.key() == key)) {
            int note = it.value();
            if (note < m_keys.size())
                return m_keys[note];
        }
    }
    return NULL;
}

void PianoScene::keyPressEvent ( QKeyEvent * keyEvent )
{
    if (!keyEvent->isAutoRepeat()) { // ignore auto-repeats 
        PianoKey* key = getPianoKey(keyEvent->key());
        if (key != NULL) {
            keyOn(key);
        }
    }
    keyEvent->accept();
}

void PianoScene::keyReleaseEvent ( QKeyEvent * keyEvent )
{
    if (!keyEvent->isAutoRepeat()) { // ignore auto-repeats
        PianoKey* key = getPianoKey(keyEvent->key());
        if (key != NULL) {
            keyOff(key);
        }
    }   
    keyEvent->accept();
}

void PianoScene::allKeysOff()
{
    QList<PianoKey*>::ConstIterator it; 
    for(it = m_keys.begin(); it != m_keys.end(); ++it) {
        (*it)->setPressed(false);
    }
}

void PianoScene::setKeyPressedColor(const QColor& color)
{
    if (color.isValid() && (color != m_keyPressedColor)) {
        m_keyPressedColor = color;
        QBrush hilightBrush(m_keyPressedColor);
        foreach(PianoKey* key, m_keys) {
            key->setPressedBrush(hilightBrush);
        }
    }
}

void PianoScene::hideOrShowKeys()
{
    QListIterator<PianoKey*> it(m_keys);
    while(it.hasNext()) {
        PianoKey* key = it.next();
        int n = m_baseOctave*12 + key->getNote();
        bool b = !(n > m_maxNote) && !(n < m_minNote);
        key->setVisible(b);
    }
}

void PianoScene::setMinNote(const int note)
{
    m_minNote = note;
    hideOrShowKeys();
}

void PianoScene::setMaxNote(const int note)
{
    m_maxNote = note;
    hideOrShowKeys();
}

void PianoScene::setBaseOctave(const int base)
{ 
    m_baseOctave = base;
    hideOrShowKeys();
}
