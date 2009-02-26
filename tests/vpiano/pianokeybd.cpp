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

#include "pianokeybd.h"
#include "pianoscene.h"
#include <QApplication>

PianoKeybd::PianoKeybd(QWidget *parent)
    : QGraphicsView(parent), m_rotation(0)
{
    initialize();
    initScene(3, 5);
}

PianoKeybd::PianoKeybd(const int baseOctave, const int numOctaves, QWidget *parent)
    : QGraphicsView(parent), m_rotation(0)
{
    initialize();
    initScene(baseOctave, numOctaves);
}

void PianoKeybd::initScene(int base, int num, const QColor& c)
{
    m_scene = new PianoScene(base, num, c, this);
    m_scene->setKeyboardMap(&m_defaultMap);
    connect(m_scene, SIGNAL(noteOn(int)), SIGNAL(noteOn(int)));
    connect(m_scene, SIGNAL(noteOff(int)), SIGNAL(noteOff(int)));
    setScene(m_scene);
}

void PianoKeybd::initialize()
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(MinimalViewportUpdate);
    setRenderHints(QPainter::Antialiasing);
    setOptimizationFlag(DontClipPainter, true);
    setOptimizationFlag(DontSavePainterState, true);
    setOptimizationFlag(DontAdjustForAntialiasing, true);
    setBackgroundBrush(QApplication::palette().background());
    initDefaultMap();
}

void PianoKeybd::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
}

void PianoKeybd::showNoteOn(int midiNote)
{
    m_scene->showNoteOn(midiNote);
}

void PianoKeybd::showNoteOff(int midiNote)
{
    m_scene->showNoteOff(midiNote);
}

void PianoKeybd::initDefaultMap()
{
    m_defaultMap.insert(Qt::Key_Z, 12);
    m_defaultMap.insert(Qt::Key_S, 13);
    m_defaultMap.insert(Qt::Key_X, 14);
    m_defaultMap.insert(Qt::Key_D, 15);
    m_defaultMap.insert(Qt::Key_C, 16);
    m_defaultMap.insert(Qt::Key_V, 17);
    m_defaultMap.insert(Qt::Key_G, 18);
    m_defaultMap.insert(Qt::Key_B, 19);
    m_defaultMap.insert(Qt::Key_H, 20);
    m_defaultMap.insert(Qt::Key_N, 21);
    m_defaultMap.insert(Qt::Key_J, 22);
    m_defaultMap.insert(Qt::Key_M, 23);

    m_defaultMap.insert(Qt::Key_Q, 24);
    m_defaultMap.insert(Qt::Key_2, 25);
    m_defaultMap.insert(Qt::Key_W, 26);
    m_defaultMap.insert(Qt::Key_3, 27);
    m_defaultMap.insert(Qt::Key_E, 28);
    m_defaultMap.insert(Qt::Key_R, 29);
    m_defaultMap.insert(Qt::Key_5, 30);
    m_defaultMap.insert(Qt::Key_T, 31);
    m_defaultMap.insert(Qt::Key_6, 32);
    m_defaultMap.insert(Qt::Key_Y, 33);
    m_defaultMap.insert(Qt::Key_7, 34);
    m_defaultMap.insert(Qt::Key_U, 35);
    m_defaultMap.insert(Qt::Key_I, 36);
    m_defaultMap.insert(Qt::Key_9, 37);
    m_defaultMap.insert(Qt::Key_O, 38);
    m_defaultMap.insert(Qt::Key_0, 39);
    m_defaultMap.insert(Qt::Key_P, 40);
}

void PianoKeybd::setNumOctaves(const int numOctaves)
{
    if (numOctaves != m_scene->numOctaves()) {
        int baseOctave = m_scene->baseOctave();
        QColor color = m_scene->getKeyPressedColor();
        PianoHandler* handler = m_scene->getPianoHandler();
        KeyboardMap* keyMap = m_scene->getKeyboardMap();
        delete m_scene;
        initScene(baseOctave, numOctaves, color);
        m_scene->setPianoHandler(handler);
        m_scene->setKeyboardMap(keyMap);
        fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
    }
}

void PianoKeybd::setRotation(int r)
{
    if (r != m_rotation) {
        m_rotation = r;
        resetTransform();
        rotate(m_rotation);
        fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
    }
}

QSize PianoKeybd::sizeHint() const
{
    return mapFromScene(sceneRect()).boundingRect().size();
}
