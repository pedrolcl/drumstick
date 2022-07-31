/*
    Virtual Piano Widget for Qt5
    Copyright (C) 2008-2022, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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
#include "pianoscene.h"

/**
 * @file pianokeybd.cpp
 * Implementation of the PianoKeybd class
 */

/**
 * @class QGraphicsView
 * The QGraphicsView class provides a widget for displaying the contents of a QGraphicsScene.
 * @see https://doc.qt.io/qt-5/qgraphicsview.html
 */

namespace drumstick { namespace widgets {

/**
 * Global Default Alphanumeric Keyboard Map
 */
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

/**
 * Global Default Raw Keyboard Map
 */
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
    ~PianoKeybdPrivate() = default;

    int m_rotation;
    PianoScene *m_scene;
    KeyboardMap *m_rawMap;
};

/**
 * @brief Constructor
 *
 * This is the usual constructor when using QtDesigner and without providing
 * custom settings, using the default octave, number of keys and starting key.
 * @param parent Widget's parent
 */
PianoKeybd::PianoKeybd(QWidget *parent) 
    : QGraphicsView(parent), d(new PianoKeybdPrivate())
{
    initialize();
    initScene(DEFAULTBASEOCTAVE, DEFAULTNUMBEROFKEYS, DEFAULTSTARTINGKEY);
}

/**
 * @brief Constructor providing not only a parent widget, but also
 * custom values for octave, number of keys and starting key.
 * @param baseOctave The base octave number
 * @param numKeys The number of displayed keys
 * @param startKey The startup key
 * @param parent The widget's parent
 */
PianoKeybd::PianoKeybd(const int baseOctave, const int numKeys, const int startKey, QWidget *parent)
    : QGraphicsView(parent), d(new PianoKeybdPrivate)
{
    initialize();
    initScene(baseOctave, numKeys, startKey);
}

/**
 * @brief Destructor
 */
PianoKeybd::~PianoKeybd()
{
    d->m_scene->setRawKeyboardMode(false);
    setKeyboardMap(nullptr);
}

/**
 * Gets the PianoHandler pointer to the note receiver.
 *
 * If this method returns null, then there is not a PianoHandler class assigned,
 * and then the signals noteOn() and noteOff() are emitted instead.
 * @return pointer to the PianoHandler class, if there is one assigned
 */
PianoHandler *PianoKeybd::getPianoHandler() const
{
    return d->m_scene->getPianoHandler();
}

/**
 * Assigns a PianoHandler pointer for processing note events.
 *
 * When this member is used to assign a PianoHandler instance, then
 * the methods in that instance are called instead of emitting the
 * signals noteOn() and noteOff().
 * @param handler pointer to the PianoHandler instance
 */
void PianoKeybd::setPianoHandler(PianoHandler *handler)
{
    d->m_scene->setPianoHandler(handler);
}

/**
 * Returns the palette used for highlighting the played keys
 * @return The PianoPalette used to highlight the played keys
 */
PianoPalette PianoKeybd::getHighlightPalette() const
{
    return d->m_scene->getHighlightPalette();
}

/**
 * Assigns the palette used for highlighting the played keys. When the palette
 * has a single color, the method setKeyPressedColor() may be used instead.
 * @see setKeyPressedColor()
 * @param p PianoPalette const reference
 */
void PianoKeybd::setHighlightPalette(const PianoPalette& p)
{
    d->m_scene->setHighlightPalette(p);
}

/**
 * Returns the palette used to paint the keys' background.
 * @return The PianoPalette used to paint the keys' background
 */
PianoPalette PianoKeybd::getBackgroundPalette() const
{
    return d->m_scene->getBackgroundPalette();
}

/**
 * Assigns the palette used to paint the keys' background.
 * @param p PianoPalette const reference
 */
void PianoKeybd::setBackgroundPalette(const PianoPalette& p)
{
    d->m_scene->setBackgroundPalette(p);
}

/**
 * Returns the palette used to paint texts over the keys like the note names
 * or custom labels.
 * @return The PianoPalette used to paint the keys' foreground
 */
PianoPalette PianoKeybd::getForegroundPalette() const
{
    return d->m_scene->getForegroundPalette();
}

/**
 * Assigns the palette used to paint texts over the keys like the note names
 * or custom labels.
 * @param p PianoPalette const reference
 */
void PianoKeybd::setForegroundPalette(const PianoPalette &p)
{
    d->m_scene->setForegroundPalette(p);
}

/**
 * Returns true if the color scale background palette is assigned and active.
 * @return whether the color scale display is enabled or not
 */
bool PianoKeybd::showColorScale() const
{
    return d->m_scene->showColorScale();
}

/**
 * Enables or disables the color scale background palette.
 * @param show the color scale activation state
 */
void PianoKeybd::setShowColorScale(const bool show)
{
    d->m_scene->setShowColorScale(show);
}

/**
 * Assigns a list of custom text labels to be displayer over the keys.
 *
 * This can be used for instance, to display the names of the percussion sounds for
 * General MIDI channel 10. The number of elements should be 128, when
 * naming the whole set of MIDI notes, or at least 12 when naming the note names,
 * in which case an octave designation may be attached to the supplied name.
 * @param names list of key labels
 */
void PianoKeybd::useCustomNoteNames(const QStringList &names)
{
    d->m_scene->useCustomNoteNames(names);
}

/**
 * Disables the custom note names usage as labels over the keys, and restores
 * the standard note names instead.
 */
void PianoKeybd::useStandardNoteNames()
{
    d->m_scene->useStandardNoteNames();
}

/**
 * Returns the list of custom note names.
 * @return the list of custom key names
 */
QStringList PianoKeybd::customNoteNames() const
{
    return d->m_scene->customNoteNames();
}

/**
 * Returns the list of standard note names.
 * @return the list of standard key names
 */
QStringList PianoKeybd::standardNoteNames() const
{
    return d->m_scene->standardNoteNames();
}

/**
 * Updates the standard names of notes according to the
 * currently active program language translation.
 * The custom note names are not affected.
 */
void PianoKeybd::retranslate()
{
    d->m_scene->retranslate();
}

/**
 * Creates and initializes a new PianoScene instance and assigns it to this widget.
 * @param base octave base number
 * @param num number of displayed keys
 * @param strt starting note number
 * @param c single default highlight color
 */
void PianoKeybd::initScene(int base, int num, int strt, const QColor& c)
{
    d->m_scene = new PianoScene(base, num, strt, c, this);
    d->m_scene->setKeyboardMap(&g_DefaultKeyMap);
    connect(d->m_scene, &PianoScene::noteOn, this, &PianoKeybd::noteOn);
    connect(d->m_scene, &PianoScene::noteOff, this, &PianoKeybd::noteOff);
    connect(d->m_scene, &PianoScene::signalName, this, &PianoKeybd::signalName);
    setScene(d->m_scene);
}

/**
 * This method is called from the available constructors
 * to initialize some widget attributes, settings, and optimizations.
 */
void PianoKeybd::initialize()
{
    setAttribute(Qt::WA_AcceptTouchEvents);
    setAttribute(Qt::WA_InputMethodEnabled, false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setViewportUpdateMode(MinimalViewportUpdate);
    setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform);
#if (QT_VERSION < QT_VERSION_CHECK(5,15,0))
    setOptimizationFlag(DontClipPainter, true);
#endif
    setOptimizationFlag(DontSavePainterState, true);
    setOptimizationFlag(DontAdjustForAntialiasing, true);
}

/**
 * This method overrides QGraphicsView::resizeEvent()
 * to keep the aspect ratio of the keys scene when the view is resized.
 * @param event
 */
void PianoKeybd::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    fitInView(d->m_scene->sceneRect(), Qt::KeepAspectRatio);
}

/**
 * This method overrides QGraphicsView::viewportEvent()
 * Only touchscreen events are processed here.
 * @param ev The viewport event
 * @return true if the event has been consumed, false otherwise
 */
bool PianoKeybd::viewportEvent(QEvent *ev)
{
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
    static const auto touchScreen = QTouchDevice::DeviceType::TouchScreen;
#else
    static const auto touchScreen = QInputDevice::DeviceType::TouchScreen;
#endif
    switch(ev->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
    {
        //qDebug() << Q_FUNC_INFO << ev->type();
        QTouchEvent *touchEvent = static_cast<QTouchEvent*>(ev);
        if (isTouchEnabled() && (touchEvent->device()->type() == touchScreen)) {
            return d->m_scene->touchScreenEvent(touchEvent);
        }
        break;
    }
    default:
        break;
    }
    return QGraphicsView::viewportEvent(ev);
}

/**
 * This method changes the number of displayed keys
 * and the starting key number, keeping the other settings the same.
 * The common industrial piano layout has 88 keys, and starts with A, so
 * the default starting key value is 9. But any natural note/key
 * number is possible as starting note key.
 * The key/note values are: C=0, D=2, E=4, F=5, G=7, A=9, B=11.
 * @see numKeys(), startKey(), setStartKey()
 * @param numKeys The new number of keys
 * @param startKey The number of the starting key
 */
void PianoKeybd::setNumKeys(const int numKeys, const int startKey)
{
    //qDebug() << Q_FUNC_INFO << numKeys << startKey;
    if ( numKeys != d->m_scene->numKeys() || startKey != d->m_scene->startKey() )
    {
        QByteArray dataBuffer;
        int baseOctave = d->m_scene->baseOctave();
        QColor color = d->m_scene->getKeyPressedColor();
        PianoHandler* handler = d->m_scene->getPianoHandler();
        KeyboardMap* keyMap = d->m_scene->getKeyboardMap();
        d->m_scene->saveData(dataBuffer);
        delete d->m_scene;
        initScene(baseOctave, numKeys, startKey, color);
        d->m_scene->loadData(dataBuffer);
        d->m_scene->setPianoHandler(handler);
        d->m_scene->setKeyboardMap(keyMap);
        d->m_scene->hideOrShowKeys();
        d->m_scene->refreshKeys();
        d->m_scene->refreshLabels();
        fitInView(d->m_scene->sceneRect(), Qt::KeepAspectRatio);
    }
}

/**
 * Rotates the keyboard view an angle clockwise.
 * @param r rotating angle in degrees to rotate.
 */
void PianoKeybd::setRotation(int r)
{
    if (r != d->m_rotation) {
        d->m_rotation = r;
        resetTransform();
        rotate(d->m_rotation);
        fitInView(d->m_scene->sceneRect(), Qt::KeepAspectRatio);
    }
}

/**
 * Overrides QGraphicsView::sizeHint() providing a size value based on the piano scene.
 * @return The sizeHint property of the piano view
 */
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

/**
 * @brief Assigns a custom picture to the white or black keys
 * that will be used as a texture to paint the keys.
 * @param natural true for white keys, false for black keys
 * @param pix this transparent QPixmap will be used to paint the keys
 */
void PianoKeybd::setKeyPicture(const bool natural, const QPixmap &pix)
{
    d->m_scene->setKeyPicture(natural, pix);
}

/**
 * @brief Returns the custom picture used to paint the corresponding keys.
 * @param natural true for white keys, false for black keys
 * @return a transparent QPixmap used to paint the keys
 */
QPixmap PianoKeybd::getKeyPicture(const bool natural)
{
    return d->m_scene->getKeyPicture(natural);
}

/**
 * @brief Enables or disables a picture to paint the keys.
 * @param enable or disable
 */
void PianoKeybd::setUseKeyPictures(const bool enable)
{
    d->m_scene->setUseKeyPictures(enable);
}

/**
 * @brief Returns whether pictures are used to paint the keys.
 * @return  true if the pictures are enabled to paint the keys
 */
bool PianoKeybd::getUseKeyPictures() const
{
    return d->m_scene->getUseKeyPictures();
}

/**
 * @brief Enables or disables the application level usage of a native event filter
 *
 * The native event filter should process low level keyboard events,
 * calling the methods of the @ref RawKbdHandler interface. This method should be
 * used in this case to indicate that the keyboard events should be ignored
 * by the piano scene keyboard event handlers.
 * Note: this is only necessary if the native filter does not block events.
 * @param newState of the application level usage of a native event filter
 */
void PianoKeybd::setUsingNativeFilter(const bool newState)
{
    d->m_scene->setUsingNativeFilter( newState );
}

/**
 * @brief Returns whether the application is filtering native events
 *
 * The native event filter should process low level keyboard events,
 * calling the methods of the @ref RawKbdHandler interface.
 * @return true if the application is filtering native events
 */
bool PianoKeybd::isUsingNativeFilter() const
{
    return d->m_scene->isUsingNativeFilter();
}

/**
 * @brief Enables or disables the octave subscript designation
 *
 * According to the Scientific pitch notation (SPN), also known as
 * American standard pitch notation (ASPN), the octave designation
 * should be written as a subscript, but it is an user choice.
 * 
 * @see octaveSubscript()
 * @param enable or disable using subscript octave numbers
 * @since 2.7.0
 */
void PianoKeybd::setOctaveSubscript(const bool enable)
{
    d->m_scene->setOctaveSubscript( enable );
}

/**
 * @brief Returns whether the octave subscript designation is enabled
 *
 * According to the Scientific pitch notation (SPN), also known as
 * American standard pitch notation (ASPN), the octave designation
 * should be written as a subscript.
 * @see setOctaveSubscript()
 * @return true if the octave subscript designation is enabled
 * @since 2.7.0
 */
bool PianoKeybd::octaveSubscript() const
{
    return d->m_scene->octaveSubscript();
}

/** 
 * @brief Sets the initial/starting note key
 * 
 * The common industrial piano keys layout (88 keys) starts with A, so
 * the default starting key value is 9. But any natural note/key
 * number is possible as starting note key.
 * The key/note values are: C=0, D=2, E=4, F=5, G=7, A=9, B=11.
 * @see setNumKeys()
 * @since 2.7.0
 * @param startKey is a note/key number between 0 and 11
 */
void PianoKeybd::setStartKey(const int startKey)
{
    setNumKeys(numKeys(), startKey);
}

/**
 * Returns the base octave number.
 * @see setBaseOctave()
 * @return the base octave number
 */
int PianoKeybd::baseOctave() const
{
    return d->m_scene->baseOctave();
}

/**
 * Assigns the base octave number.
 * @param baseOctave the base octave number
 */
void PianoKeybd::setBaseOctave(const int baseOctave)
{
    d->m_scene->setBaseOctave(baseOctave);
}

/**
 * Returns the total number of keys
 * @see setNumKeys()
 * @return the number of keys displayed
 */
int PianoKeybd::numKeys() const
{
    return d->m_scene->numKeys();
}

/**
 * Returns the starting key note: C=0, A=9 and so on.
 * @return the starting key note.
 */
int PianoKeybd::startKey() const
{
    return d->m_scene->startKey();
}

/**
 * Returns the rotation angle in degrees, clockwise, of the piano view.
 * @return the rotation angle in degrees.
 */
int PianoKeybd::getRotation() const
{
    return d->m_rotation;
}

/**
 * Returns the key highlight color.
 * @return the key highlight color
 */
QColor PianoKeybd::getKeyPressedColor() const
{
    return d->m_scene->getKeyPressedColor();
}

/**
 * Assigns a single color for key highlight. This is an alternative to creating a
 * highlight palette with a single color and assigning it.
 * @see setHighlightPalette()
 * @param c color for key highlight
 */
void PianoKeybd::setKeyPressedColor(const QColor& c)
{
    d->m_scene->setKeyPressedColor(c);
}

/**
 * Assigns the default highlight palette colors and assigns it to the scene.
 */
void PianoKeybd::resetKeyPressedColor()
{
    d->m_scene->resetKeyPressedColor();
}

/**
 * Returns the label visibility policy.
 * @see setShowLabels()
 * @return the label visibility policy
 */
LabelVisibility PianoKeybd::showLabels() const
{
    return d->m_scene->showLabels();
}

/**
 * Assigns the label visibility policy.
 * @see LabelVisibility
 * @param show the label visibility policy
 */
void PianoKeybd::setShowLabels(const LabelVisibility show)
{
    d->m_scene->setShowLabels(show);
}

/**
 * Returns the label alterations policy.
 * @see setLabelAlterations()
 * @return the label alterations policy
 */
LabelAlteration PianoKeybd::labelAlterations() const
{
    return d->m_scene->alterations();
}

/**
 * Assigns the label alterations policy.
 * @see LabelAlteration
 * @param use the label alterations policy
 */
void PianoKeybd::setLabelAlterations(const LabelAlteration use)
{
    d->m_scene->setAlterations(use);
}

/**
 * Returns the labels orientation policy.
 * @see setLabelOrientation()
 * @return the labels orientation policy
 */
LabelOrientation PianoKeybd::labelOrientation() const
{
    return d->m_scene->getOrientation();
}

/**
 * Assigns the labels orientation policy.
 * @see LabelOrientation
 * @param orientation the labels orientation policy
 */
void PianoKeybd::setLabelOrientation(const LabelOrientation orientation)
{
    d->m_scene->setOrientation(orientation);
}

/**
 * Returns the octave label policy.
 * @see setLabelOctave()
 * @return the octave label policy
 */
LabelCentralOctave PianoKeybd::labelOctave() const
{
    return d->m_scene->getOctave();
}

/**
 * Assigns the octave label policy
 * @see LabelCentralOctave
 * @param octave the octave label policy
 */
void PianoKeybd::setLabelOctave(const LabelCentralOctave octave)
{
    d->m_scene->setOctave(octave);
}

/**
 * Returns the transpose amount in semitones.
 * @see setTranspose()
 * @return the transpose amount in semitones
 */
int PianoKeybd::getTranspose() const
{
    return d->m_scene->getTranspose();
}

/**
 * Assigns the transpose amount in semitones.
 * @see getTranspose()
 * @param t the transpose amount in semitones
 */
void PianoKeybd::setTranspose(int t)
{
    d->m_scene->setTranspose(t);
}

/**
 * Returns the MIDI Channel (0-15).
 * @return the MIDI Channel (0-15)
 */
int PianoKeybd::getChannel() const
{
    return d->m_scene->getChannel();
}

/**
 * Assigns the MIDI Channel (0-15).
 * @param c the MIDI Channel (0-15)
 */
void PianoKeybd::setChannel(const int c)
{
    d->m_scene->setChannel(c);
}

/**
 * Returns the MIDI note velocity
 * @see setVelocity()
 * @return  the MIDI note velocity
 */
int PianoKeybd::getVelocity() const
{
    return d->m_scene->getVelocity();
}

/**
 * Assigns the MIDI note velocity.
 * @see getVelocity()
 * @param v the MIDI note velocity
 */
void PianoKeybd::setVelocity(const int v)
{
    d->m_scene->setVelocity(v);
}

/**
 * Returns whether the computer keyboard is enabled.
 * @return true if the computer keyboard is enabled.
 */
bool PianoKeybd::isKeyboardEnabled() const
{
    return d->m_scene->isKeyboardEnabled();
}

/**
 * Enables or disables the computer keyboard note input.
 * @param enable the computer keyboard note input.
 */
void PianoKeybd::setKeyboardEnabled(const bool enable)
{
    d->m_scene->setKeyboardEnabled(enable);
}

/**
 * Returns whether the mouse note input is enabled.
 * @return true if the mouse note input is enabled
 */
bool PianoKeybd::isMouseEnabled() const
{
    return d->m_scene->isMouseEnabled();
}

/**
 * Enables or disables the mouse note input.
 * @param enable the mouse note input
 */
void PianoKeybd::setMouseEnabled(const bool enable)
{
    d->m_scene->setMouseEnabled(enable);
}

/**
 * Returns whether the touch screen note input is enabled.
 * @return true if the touch screen note input is enabled
 */
bool PianoKeybd::isTouchEnabled() const
{
    return d->m_scene->isTouchEnabled();
}

/**
 * Enables or disables the touch screen note input.
 * @param enable the touch screen note input.
 */
void PianoKeybd::setTouchEnabled(const bool enable)
{
    d->m_scene->setTouchEnabled(enable);
}

/**
 * Returns whether the note MIDI velocity influences the highlight color tint.
 * @return true if the note MIDI velocity influences the highlight color tint
 */
bool PianoKeybd::velocityTint() const
{
    return d->m_scene->velocityTint();
}

/**
 * Enables or disables the note MIDI velocity influencing the highlight color tint.
 * @param enable the note MIDI velocity influencing the highlight color tint
 */
void PianoKeybd::setVelocityTint(const bool enable)
{
    //qDebug() << Q_FUNC_INFO << enable;
    d->m_scene->setVelocityTint(enable);
}

/**
 * Forces all active notes to silence.
 */
void PianoKeybd::allKeysOff()
{
    d->m_scene->allKeysOff();
}

/**
 * Assigns the computer keyboard note map.
 * @param m the computer keyboard note map.
 */
void PianoKeybd::setKeyboardMap(KeyboardMap* m)
{
    d->m_scene->setKeyboardMap(m);
}

/**
 * Returns the computer keyboard note map.
 * @return the computer keyboard note map
 */
KeyboardMap* PianoKeybd::getKeyboardMap()
{
    return d->m_scene->getKeyboardMap();
}

/**
 * Resets the computer keyboard note map to the default one.
 */
void PianoKeybd::resetRawKeyboardMap()
{
    d->m_rawMap = &g_DefaultRawKeyMap;
    d->m_scene->setKeyboardMap(&g_DefaultRawKeyMap);
}

/**
 * Returns the low level computer keyboard note map.
 * @return the low level computer keyboard note map
 */
bool PianoKeybd::getRawKeyboardMode() const
{
    return d->m_scene->getRawKeyboardMode();
}

/**
 * Enables or disables the low level computer keyboard mode.
 * @param b the low level computer keyboard mode
 */
void PianoKeybd::setRawKeyboardMode(const bool b)
{
    d->m_scene->setRawKeyboardMode(b);
}

/**
 * Resets the low level computer keyboard note map to the default one.
 */
void PianoKeybd::resetKeyboardMap()
{
    d->m_scene->setKeyboardMap(&g_DefaultKeyMap);
}

/**
 * Assigns the low level computer keyboard note map.
 * @param m the low level computer keyboard note map
 */
void PianoKeybd::setRawKeyboardMap(KeyboardMap *m)
{
    d->m_rawMap = m;
    d->m_scene->setKeyboardMap(m);
}

/**
 * Returns the low level computer keyboard note map.
 * @return the low level computer keyboard note map
 */
KeyboardMap *PianoKeybd::getRawKeyboardMap()
{
    return d->m_rawMap;
}

/**
 * Highlights one note key with the specified color and velocity
 * @param note The MIDI note number
 * @param color The highlight color
 * @param vel The MIDI note velocity
 */
void PianoKeybd::showNoteOn(const int note, QColor color, int vel)
{
    d->m_scene->showNoteOn(note, color, vel);
}

/**
 * Highlights one note key with the specified velocity
 * @param note The MIDI note number
 * @param vel The MIDI note velocity
 */
void PianoKeybd::showNoteOn(const int note, int vel)
{
    d->m_scene->showNoteOn(note, vel);
}

/**
 * Shows inactive one note key with the specified velocity
 * @param note The MIDI note number
 * @param vel The MIDI note velocity
 */
void PianoKeybd::showNoteOff(const int note, int vel)
{
    d->m_scene->showNoteOff(note, vel);
}

/**
 * Assigns a typographic font for drawing the note labels over the piano keys.
 * @param font typographic font for drawing the note labels
 */
void PianoKeybd::setFont(const QFont &font)
{
    QWidget::setFont(font);
    d->m_scene->setFont(font);
    d->m_scene->refreshLabels();
}

} // namespace widgets
} // namespace drumstick
