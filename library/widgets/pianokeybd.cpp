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
#include <drumstick/pianokeybd.h>
#include <drumstick/pianoscene.h>

namespace drumstick {
namespace widgets {

KeyboardMap g_DefaultKeyMap {
    {Qt::Key_Z, 12},
    {Qt::Key_S, 13},
    {Qt::Key_X, 14},
    {Qt::Key_D, 15},
    {Qt::Key_C, 16},
    {Qt::Key_V, 17},
    {Qt::Key_G, 18},
    {Qt::Key_B, 19},
    {Qt::Key_H, 20},
    {Qt::Key_N, 21},
    {Qt::Key_J, 22},
    {Qt::Key_M, 23},
    {Qt::Key_Q, 24},
    {Qt::Key_2, 25},
    {Qt::Key_W, 26},
    {Qt::Key_3, 27},
    {Qt::Key_E, 28},
    {Qt::Key_R, 29},
    {Qt::Key_5, 30},
    {Qt::Key_T, 31},
    {Qt::Key_6, 32},
    {Qt::Key_Y, 33},
    {Qt::Key_7, 34},
    {Qt::Key_U, 35},
    {Qt::Key_I, 36},
    {Qt::Key_9, 37},
    {Qt::Key_O, 38},
    {Qt::Key_0, 39},
    {Qt::Key_P, 40}
};

KeyboardMap g_DefaultRawKeyMap {
#if defined(Q_OS_LINUX)
    {94, 11},
    {52, 12},
    {39, 13},
    {53, 14},
    {40, 15},
    {54, 16},
    {55, 17},
    {42, 18},
    {56, 19},
    {43, 20},
    {57, 21},
    {44, 22},
    {58, 23},
    {59, 24},
    {46, 25},
    {60, 26},
    {47, 27},
    {61, 28},

    {24, 29},
    {11, 30},
    {25, 31},
    {12, 32},
    {26, 33},
    {13, 34},
    {27, 35},
    {28, 36},
    {15, 37},
    {29, 38},
    {16, 39},
    {30, 40},
    {31, 41},
    {18, 42},
    {32, 43},
    {19, 44},
    {33, 45},
    {20, 46},
    {34, 47},
    {35, 48}
#endif

#if defined(Q_OS_WIN)
    {86, 11},
    {44, 12},
    {31, 13},
    {45, 14},
    {32, 15},
    {46, 16},
    {47, 17},
    {34, 18},
    {48, 19},
    {35, 20},
    {49, 21},
    {36, 22},
    {50, 23},
    {51, 24},
    {38, 25},
    {52, 26},
    {39, 27},
    {53, 28},

    {16, 29},
    {3, 30},
    {17, 31},
    {4, 32},
    {18, 33},
    {5, 34},
    {19, 35},
    {20, 36},
    {7, 37},
    {21, 38},
    {8, 39},
    {22, 40},
    {23, 41},
    {10, 42},
    {24, 43},
    {11, 44},
    {25, 45},
    {12, 46},
    {26, 47},
    {27, 48}
#endif

#if defined(Q_OS_MAC)
    {50, 11},
    {6, 12},
    {1, 13},
    {7, 14},
    {2, 15},
    {8, 16},
    {9, 17},
    {5, 18},
    {11, 19},
    {4, 20},
    {45, 21},
    {38, 22},
    {46, 23},
    {43, 24},
    {37, 25},
    {47, 26},
    {41, 27},
    {44, 28},

    {12, 29},
    {19, 30},
    {13, 31},
    {20, 32},
    {14, 33},
    {21, 34},
    {15, 35},
    {17, 36},
    {22, 37},
    {16, 38},
    {26, 39},
    {32, 40},
    {34, 41},
    {25, 42},
    {31, 43},
    {29, 44},
    {35, 45},
    {27, 46},
    {33, 47},
    {30, 48}
#endif
};

class PianoKeybd::PianoKeybdPrivate {
public:
    PianoKeybdPrivate(): m_rotation(0), m_scene(nullptr), m_rawMap(nullptr)
    { }

    ~PianoKeybdPrivate()
    { }

    int m_rotation;
    PianoScene *m_scene;
    KeyboardMap *m_rawMap;
};

PianoKeybd::PianoKeybd(QWidget *parent) 
    : QGraphicsView(parent), d(new PianoKeybdPrivate())
{
    initialize();
    initScene(DEFAULTBASEOCTAVE, DEFAULTNUMBEROFKEYS, DEFAULTSTARTINGKEY);
}

PianoKeybd::PianoKeybd(const int baseOctave, const int numKeys, const int startKey, QWidget *parent)
    : QGraphicsView(parent), d(new PianoKeybdPrivate)
{
    initialize();
    initScene(baseOctave, numKeys, startKey);
}

PianoKeybd::~PianoKeybd()
{
    d->m_scene->setRawKeyboardMode(false);
    setKeyboardMap(nullptr);
    delete d;
}

PianoHandler *PianoKeybd::getPianoHandler() const
{
    return d->m_scene->getPianoHandler();
}

void PianoKeybd::setPianoHandler(PianoHandler *handler)
{
    d->m_scene->setPianoHandler(handler);
}

PianoPalette& PianoKeybd::getPianoPalette() const
{
    return d->m_scene->getPianoPalette();
}

void PianoKeybd::setPianoPalette(const PianoPalette& p)
{
    d->m_scene->setPianoPalette(p);
}

void PianoKeybd::setColorScalePalette(const PianoPalette& p)
{
    d->m_scene->setColorScalePalette(p);
}

bool PianoKeybd::showColorScale() const
{
    return d->m_scene->showColorScale();
}

void PianoKeybd::setShowColorScale(const bool show)
{
    d->m_scene->setShowColorScale(show);
}

void PianoKeybd::useCustomNoteNames(const QStringList &names)
{
    d->m_scene->useCustomNoteNames(names);
}

void PianoKeybd::useStandardNoteNames()
{
    d->m_scene->useStandardNoteNames();
}

QStringList PianoKeybd::noteNames() const
{
    return d->m_scene->noteNames();
}

void PianoKeybd::retranslate()
{
    d->m_scene->retranslate();
}

void PianoKeybd::initScene(int base, int num, int strt, const QColor& c)
{
    d->m_scene = new PianoScene(base, num, strt, c, this);
    d->m_scene->setKeyboardMap(&g_DefaultKeyMap);
    connect(d->m_scene, &PianoScene::noteOn, this, &PianoKeybd::noteOn);
    connect(d->m_scene, &PianoScene::noteOff, this, &PianoKeybd::noteOff);
    connect(d->m_scene, &PianoScene::signalName, this, &PianoKeybd::signalName);
    setScene(d->m_scene);
}

void PianoKeybd::initialize()
{
    setAttribute(Qt::WA_AcceptTouchEvents);
    setAttribute(Qt::WA_InputMethodEnabled, false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(MinimalViewportUpdate);
    setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform);
    setOptimizationFlag(DontClipPainter, true);
    setOptimizationFlag(DontSavePainterState, true);
    setOptimizationFlag(DontAdjustForAntialiasing, true);
    setBackgroundBrush(QApplication::palette().window());
    resetRawKeyboardMap();
}

void PianoKeybd::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    fitInView(d->m_scene->sceneRect(), Qt::KeepAspectRatio);
}


void PianoKeybd::setNumKeys(const int numKeys, const int startKey)
{
    if ( numKeys != d->m_scene->numKeys() || startKey != d->m_scene->startKey() )
    {
        int baseOctave = d->m_scene->baseOctave();
        QColor color = d->m_scene->getKeyPressedColor();
        PianoHandler* handler = d->m_scene->getPianoHandler();
        KeyboardMap* keyMap = d->m_scene->getKeyboardMap();
        PianoPalette palette = d->m_scene->getPianoPalette();
        bool keyboardEnabled = d->m_scene->isKeyboardEnabled();
        bool mouseEnabled = d->m_scene->isMouseEnabled();
        bool touchEnabled = d->m_scene->isTouchEnabled();
        PianoKeybd::LabelVisibility showLabels = d->m_scene->showLabels();
        PianoKeybd::LabelAlteration alteration = d->m_scene->alterations();
        PianoKeybd::LabelCentralOctave octave  = d->m_scene->getOctave();
        PianoKeybd::LabelOrientation orientation = d->m_scene->getOrientation();
        delete d->m_scene;
        initScene(baseOctave, numKeys, startKey, color);
        d->m_scene->setPianoHandler(handler);
        d->m_scene->setKeyboardMap(keyMap);
        d->m_scene->setPianoPalette(palette);
        d->m_scene->setKeyboardEnabled(keyboardEnabled);
        d->m_scene->setMouseEnabled(mouseEnabled);
        d->m_scene->setTouchEnabled(touchEnabled);
        d->m_scene->setShowLabels(showLabels);
        d->m_scene->setAlterations(alteration);
        d->m_scene->setOctave(octave);
        d->m_scene->setOrientation(orientation);
        fitInView(d->m_scene->sceneRect(), Qt::KeepAspectRatio);
    }
}

void PianoKeybd::setRotation(int r)
{
    if (r != d->m_rotation) {
        d->m_rotation = r;
        resetTransform();
        rotate(d->m_rotation);
        fitInView(d->m_scene->sceneRect(), Qt::KeepAspectRatio);
    }
}

QSize PianoKeybd::sizeHint() const 
{ 
    return mapFromScene(sceneRect()).boundingRect().size();
}

// RAWKBD_SUPPORT
bool PianoKeybd::handleKeyPressed(int keycode)
{
    if (d->m_scene->isKeyboardEnabled() && d->m_rawMap != nullptr && d->m_rawMap->contains(keycode)) {
        d->m_scene->keyOn(d->m_rawMap->value(keycode));
        return true;
    }
    return false;
}

bool PianoKeybd::handleKeyReleased(int keycode)
{
    if (d->m_scene->isKeyboardEnabled() && d->m_rawMap != nullptr && d->m_rawMap->contains(keycode)) {
        d->m_scene->keyOff(d->m_rawMap->value(keycode));
        return true;
    }
    return false;
}

int PianoKeybd::baseOctave() const
{
    return d->m_scene->baseOctave();
}

void PianoKeybd::setBaseOctave(const int baseOctave)
{
    d->m_scene->setBaseOctave(baseOctave);
}

int PianoKeybd::numKeys() const
{
    return d->m_scene->numKeys();
}

int PianoKeybd::startKey() const
{
    return d->m_scene->startKey();
}

int PianoKeybd::getRotation() const
{
    return d->m_rotation;
}

QColor PianoKeybd::getKeyPressedColor() const
{
    return d->m_scene->getKeyPressedColor();
}

void PianoKeybd::setKeyPressedColor(const QColor& c)
{
    d->m_scene->setKeyPressedColor(c);
}

void PianoKeybd::resetKeyPressedColor()
{
    d->m_scene->resetKeyPressedColor();
}

PianoKeybd::LabelVisibility PianoKeybd::showLabels() const
{
    return d->m_scene->showLabels();
}

void PianoKeybd::setShowLabels(PianoKeybd::LabelVisibility show)
{
    d->m_scene->setShowLabels(show);
}

PianoKeybd::LabelAlteration PianoKeybd::labelAlterations() const
{
    return d->m_scene->alterations();
}

void PianoKeybd::setLabelAlterations(PianoKeybd::LabelAlteration use)
{
    d->m_scene->setAlterations(use);
}

PianoKeybd::LabelOrientation PianoKeybd::labelOrientation() const
{
    return d->m_scene->getOrientation();
}

void PianoKeybd::setLabelOrientation(PianoKeybd::LabelOrientation orientation)
{
    d->m_scene->setOrientation(orientation);
}

PianoKeybd::LabelCentralOctave PianoKeybd::labelOctave() const
{
    return d->m_scene->getOctave();
}

void PianoKeybd::setLabelOctave(PianoKeybd::LabelCentralOctave octave)
{
    d->m_scene->setOctave(octave);
}

int PianoKeybd::getTranspose() const
{
    return d->m_scene->getTranspose();
}

void PianoKeybd::setTranspose(int t)
{
    d->m_scene->setTranspose(t);
}

int PianoKeybd::getChannel() const
{
    return d->m_scene->getChannel();
}

void PianoKeybd::setChannel(const int c)
{
    d->m_scene->setChannel(c);
}

int PianoKeybd::getVelocity() const
{
    return d->m_scene->getVelocity();
}

void PianoKeybd::setVelocity(const int v)
{
    d->m_scene->setVelocity(v);
}

bool PianoKeybd::isKeyboardEnabled() const
{
    return d->m_scene->isKeyboardEnabled();
}

void PianoKeybd::setKeyboardEnabled(const bool enable)
{
    d->m_scene->setKeyboardEnabled(enable);
}

bool PianoKeybd::isMouseEnabled() const
{
    return d->m_scene->isMouseEnabled();
}

void PianoKeybd::setMouseEnabled(const bool enable)
{
    d->m_scene->setMouseEnabled(enable);
}

bool PianoKeybd::isTouchEnabled() const
{
    return d->m_scene->isTouchEnabled();
}

void PianoKeybd::setTouchEnabled(const bool enable)
{
    d->m_scene->setTouchEnabled(enable);
}

bool PianoKeybd::velocityTint() const
{
    return d->m_scene->velocityTint();
}

void PianoKeybd::setVelocityTint(const bool enable)
{
    d->m_scene->setVelocityTint(enable);
}

void PianoKeybd::allKeysOff()
{
    d->m_scene->allKeysOff();
}

void PianoKeybd::setKeyboardMap(KeyboardMap* m)
{
    d->m_scene->setKeyboardMap(m);
}

KeyboardMap* PianoKeybd::getKeyboardMap()
{
    return d->m_scene->getKeyboardMap();
}

void PianoKeybd::resetRawKeyboardMap()
{
    d->m_rawMap = &g_DefaultRawKeyMap;
}

bool PianoKeybd::getRawKeyboardMode() const
{
    return d->m_scene->getRawKeyboardMode();
}

void PianoKeybd::setRawKeyboardMode(const bool b)
{
    d->m_scene->setRawKeyboardMode(b);
}

void PianoKeybd::resetKeyboardMap()
{
    d->m_scene->setKeyboardMap(&g_DefaultKeyMap);
}

void PianoKeybd::setRawKeyboardMap(KeyboardMap *m)
{
    d->m_rawMap = m;
}

KeyboardMap *PianoKeybd::getRawKeyboardMap()
{
    return d->m_rawMap;
}

void PianoKeybd::showNoteOn(const int note, QColor color, int vel)
{
    d->m_scene->showNoteOn(note, color, vel);
}

void PianoKeybd::showNoteOn(const int note, int vel)
{
    d->m_scene->showNoteOn(note, vel);
}

void PianoKeybd::showNoteOff(const int note, int vel)
{
    d->m_scene->showNoteOff(note, vel);
}

void PianoKeybd::setFont(const QFont &font)
{
    QWidget::setFont(font);
    d->m_scene->setFont(font);
    d->m_scene->refreshLabels();
}

}} // namespace drumstick::widgets
