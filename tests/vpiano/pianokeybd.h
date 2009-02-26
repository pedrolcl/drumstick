/*
    Virtual Piano Widget for Qt4
    Copyright (C) 2008-2009, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef PIANOKEYBD_H
#define PIANOKEYBD_H

#include "pianoscene.h"
#include <QGraphicsView>
#include <QtDesigner/QDesignerExportWidget>

class QDESIGNER_WIDGET_EXPORT PianoKeybd : public QGraphicsView
{
    Q_OBJECT
    Q_PROPERTY( int baseOctave READ baseOctave WRITE setBaseOctave )
    Q_PROPERTY( int numOctaves READ numOctaves WRITE setNumOctaves )
    Q_PROPERTY( int rotation READ getRotation WRITE setRotation )
    Q_PROPERTY( QColor keyPressedColor READ getKeyPressedColor WRITE setKeyPressedColor )

public:
    PianoKeybd(QWidget *parent = 0);
    PianoKeybd(const int baseOctave, const int numOctaves, QWidget *parent = 0);
    void setKeyboardMap(KeyboardMap* m) { m_scene->setKeyboardMap(m); }
    KeyboardMap* getKeyboardMap() { return m_scene->getKeyboardMap(); }
    int baseOctave() const { return m_scene->baseOctave(); }
    int numOctaves() const { return m_scene->numOctaves(); }
    void setBaseOctave(const int baseOctave) { m_scene->setBaseOctave(baseOctave); }
    void setNumOctaves(const int numOctaves);
    QSize sizeHint() const;
    int getRotation() const { return m_rotation; }
    void setRotation(int r);
    QColor getKeyPressedColor() const { return m_scene->getKeyPressedColor(); }
    void setKeyPressedColor(const QColor& c) { m_scene->setKeyPressedColor(c); }
    void allKeysOff() { m_scene->allKeysOff(); }
    void setPianoHandler(PianoHandler* handler) { m_scene->setPianoHandler(handler); }
    int minNote() const { return m_scene->getMinNote(); }
    void setMinNote(int n) { m_scene->setMinNote(n); }
    int maxNote() const { return m_scene->getMaxNote(); }
    void setMaxNote(int n) { m_scene->setMaxNote(n); }

public slots:
    void showNoteOn( int midiNote );
    void showNoteOff( int midiNote );

signals:
    void noteOn( int midiNote );
    void noteOff( int midiNote );

protected:
    void initialize();
    void initDefaultMap();
    void initScene(int base, int num, const QColor& c = QColor());
    void resizeEvent(QResizeEvent *event);

private:
    int m_rotation;
    PianoScene *m_scene;
    KeyboardMap m_defaultMap;
};

#endif // PIANOKEYBD_H
