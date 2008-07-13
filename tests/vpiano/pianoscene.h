/*
    Virtual Piano Widget for Qt4 
    Copyright (C) 2008, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along 
    with this program; if not, write to the Free Software Foundation, Inc., 
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.    
*/

#ifndef PIANOSCENE_H_
#define PIANOSCENE_H_

#include "pianokey.h"
#include <QGraphicsScene>
#include <QKeySequence>
#include <QMap>

#define KeyboardMap QMap<QKeySequence, int>  

class PianoScene : public QGraphicsScene
{
    Q_OBJECT

public:
    PianoScene ( const int baseOctave, const int numOctaves, QObject * parent = 0 );
    void setKeyboardMap( const KeyboardMap& map );
    void showNoteOn( const int note );
    void showNoteOff( const int note );
    int baseOctave() const { return m_baseOctave; }
    void setBaseOctave( const int base ) { m_baseOctave = base; }
    int numOctaves() const { return m_numOctaves; }
    QSize sizeHint() const;

signals:
    void noteOn(int n);
    void noteOff(int n);

protected:
    void showKeyOn( PianoKey* key );
    void showKeyOff( PianoKey* key );
    void keyOn( PianoKey* key );
    void keyOff( PianoKey* key );
    PianoKey* getKeyForPos( const QPointF& p );
    PianoKey* getPianoKey( const int key );

    void mouseMoveEvent ( QGraphicsSceneMouseEvent * mouseEvent );
    void mousePressEvent ( QGraphicsSceneMouseEvent * mouseEvent );
    void mouseReleaseEvent ( QGraphicsSceneMouseEvent * mouseEvent );
    void keyPressEvent ( QKeyEvent * keyEvent );
    void keyReleaseEvent ( QKeyEvent * keyEvent );

private:
    int m_baseOctave;
    int m_numOctaves;
    bool m_mousePressed;
    KeyboardMap m_keybdMap;
    QList<PianoKey*> m_keys;
};

#endif /*PIANOSCENE_H_*/
