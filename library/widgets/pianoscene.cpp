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
#include <QDataStream>
#include <QByteArray>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPalette>
#include <QPixmap>
#include <QtMath>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <QTouchDevice>
#else
#include <QInputDevice>
#endif
#include <drumstick/pianokeybd.h>
#include "pianoscene.h"

/**
 * @file pianoscene.cpp
 * Implementation of the Piano Scene
 */

/**
 * @class QGraphicsScene
 * The QGraphicsScene class provides a surface for managing a large number of 2D graphical items.
 * @see https://doc.qt.io/qt-5/qgraphicsscene.html
 */

namespace drumstick { namespace widgets {

class PianoScene::PianoScenePrivate
{
public:
    PianoScenePrivate ( const int baseOctave,
                        const int numKeys,
                        const int startKey ):
        m_baseOctave( baseOctave ),
        m_numKeys( numKeys ),
        m_startKey( startKey ),
        m_minNote( 0 ),
        m_maxNote( 127 ),
        m_transpose( 0 ),
        m_showLabels( ShowNever ),
        m_alterations( ShowSharps ),
        m_octave( OctaveC4 ),
        m_orientation( HorizontalOrientation ),
        m_rawkbd( false ),
        m_keyboardEnabled( true ),
        m_mouseEnabled( true ),
        m_touchEnabled( true ),
        m_mousePressed( false ),
        m_velocity( 100 ),
        m_channel( 0 ),
        m_velocityTint( true ),
        m_handler( nullptr ),
        m_keybdMap( nullptr ),
        m_showColorScale( false ),
        m_hilightPalette(PianoPalette(PAL_SINGLE)),
        m_backgroundPalette(PianoPalette(PAL_KEYS)),
        m_foregroundPalette(PianoPalette(PAL_FONT)),
        m_useKeyPix( true )
    { }

    void saveData(QByteArray& buffer)
    {
        QDataStream ds(&buffer, QIODevice::WriteOnly);
        ds << m_minNote;
        ds << m_maxNote;
        ds << m_transpose;
        ds << m_showLabels;
        ds << m_alterations;
        ds << m_octave;
        ds << m_orientation;
        ds << m_rawkbd;
        ds << m_keyboardEnabled;
        ds << m_mouseEnabled;
        ds << m_touchEnabled;
        ds << m_mousePressed;
        ds << m_velocity;
        ds << m_channel;
        ds << m_velocityTint;
        ds << m_noteNames;
        ds << m_names_s;
        ds << m_names_f;
        ds << m_showColorScale;
        ds << m_hilightPalette;
        ds << m_backgroundPalette;
        ds << m_foregroundPalette;
        ds << m_useKeyPix;
        ds << m_keyPix[0];
        ds << m_keyPix[1];
    }

    void loadData(QByteArray& buffer)
    {
        quint32 u;
        QDataStream ds(&buffer, QIODevice::ReadOnly);
        ds >> m_minNote;
        ds >> m_maxNote;
        ds >> m_transpose;
        ds >> u; m_showLabels = LabelVisibility(u);
        ds >> u; m_alterations = LabelAlteration(u);
        ds >> u; m_octave = LabelCentralOctave(u);
        ds >> u; m_orientation =  LabelOrientation(u);
        ds >> m_rawkbd;
        ds >> m_keyboardEnabled;
        ds >> m_mouseEnabled;
        ds >> m_touchEnabled;
        ds >> m_mousePressed;
        ds >> m_velocity;
        ds >> m_channel;
        ds >> m_velocityTint;
        ds >> m_noteNames;
        ds >> m_names_s;
        ds >> m_names_f;
        ds >> m_showColorScale;
        ds >> m_hilightPalette;
        ds >> m_backgroundPalette;
        ds >> m_foregroundPalette;
        ds >> m_useKeyPix;
        ds >> m_keyPix[0];
        ds >> m_keyPix[1];
    }

    int m_baseOctave;
    int m_numKeys;
    int m_startKey;
    int m_minNote;
    int m_maxNote;
    int m_transpose;
    LabelVisibility m_showLabels;
    LabelAlteration m_alterations;
    LabelCentralOctave m_octave;
    LabelOrientation m_orientation;
    bool m_rawkbd;
    bool m_keyboardEnabled;
    bool m_mouseEnabled;
    bool m_touchEnabled;
    bool m_mousePressed;
    int m_velocity;
    int m_channel;
    bool m_velocityTint;
    PianoHandler *m_handler;
    KeyboardMap *m_keybdMap;
    QHash<int, PianoKey *> m_keys;
    QMap<int, KeyLabel *> m_labels;
    QStringList m_noteNames;
    QStringList m_names_s;
    QStringList m_names_f;
    bool m_showColorScale;
    PianoPalette m_hilightPalette;
    PianoPalette m_backgroundPalette;
    PianoPalette m_foregroundPalette;
    bool m_useKeyPix;
    QPixmap m_keyPix[2];
};

const int KEYWIDTH = 180;
const int KEYHEIGHT = 720;

static qreal sceneWidth(int keys) {
    return KEYWIDTH * qCeil( keys * 7.0 / 12.0 );
}

/**
 * Constructor.
 * @param baseOctave octave base number
 * @param numKeys number of keys
 * @param startKey starting key
 * @param keyPressedColor highlight keys color
 * @param parent owner object
 */
PianoScene::PianoScene ( const int baseOctave,
                         const int numKeys,
                         const int startKey,
                         const QColor& keyPressedColor,
                         QObject * parent )
    : QGraphicsScene( QRectF(0, 0, sceneWidth(numKeys), KEYHEIGHT), parent ),
      d(new PianoScenePrivate(baseOctave, numKeys, startKey))
{
    if (keyPressedColor.isValid()) {
        setKeyPressedColor(keyPressedColor);
    }
    QBrush hilightBrush(getKeyPressedColor());
    PianoKeybd* view = dynamic_cast<PianoKeybd*>(parent);
    if (view != nullptr) {
        setFont(view->font());
    }
    int upperLimit = d->m_numKeys + d->m_startKey;
    int adj = d->m_startKey % 12;
    if (adj >= 5) adj++;
    for(int i = d->m_startKey; i < upperLimit; ++i)
    {
        float x = 0;
        PianoKey* key = nullptr;
        KeyLabel* lbl = nullptr;
        int ocs = i / 12 * 7;
        int j = i % 12;
        if (j >= 5) j++;
        if ((j % 2) == 0) {
            x = (ocs + qFloor((j-adj) / 2.0)) * KEYWIDTH;
            key = new PianoKey( QRectF(x, 0, KEYWIDTH, KEYHEIGHT), false, i );
            lbl = new KeyLabel(key);
            lbl->setDefaultTextColor(d->m_foregroundPalette.getColor(0));
        } else {
            x = (ocs + qFloor((j-adj) / 2.0)) * KEYWIDTH + KEYWIDTH * 0.6 + 1;
            key = new PianoKey( QRectF( x, 0, KEYWIDTH * 0.8 - 1, KEYHEIGHT * 0.6 ), true, i );
            key->setZValue( 1 );
            lbl = new KeyLabel(key);
            lbl->setDefaultTextColor(d->m_foregroundPalette.getColor(1));
        }
        addItem( key );
        lbl->setFont(font());
        key->setAcceptTouchEvents(true);
        key->setPressedBrush(hilightBrush);
        d->m_keys.insert(i, key);
        d->m_labels.insert(i, lbl);
    }
    hideOrShowKeys();
    retranslate();
}

/**
 * Destructor.
 */
PianoScene::~PianoScene()
{ }

/**
 * Returns the calculated size of the scene.
 * @return the calculated size of the scene
 */
QSize PianoScene::sizeHint() const
{
    return {static_cast<int>(sceneWidth(d->m_numKeys)), KEYHEIGHT};
}

/**
 * Assigns the computer keyboard note map.
 * @param map the computer keyboard note map.
 */
void PianoScene::setKeyboardMap(KeyboardMap *map)
{
    d->m_keybdMap = map;
}

/**
 * Returns the computer keyboard note map.
 * @return the computer keyboard note map
 */
KeyboardMap *PianoScene::getKeyboardMap() const
{
    return d->m_keybdMap;
}

/**
 * Gets the PianoHandler pointer to the note receiver.
 *
 * If this method returns null, then there is not a PianoHandler class assigned,
 * and then the signals noteOn() and noteOff() are emitted instead.
 * @return pointer to the PianoHandler class, if there is one assigned
 */
PianoHandler *PianoScene::getPianoHandler() const
{
    return d->m_handler;
}

/**
 * Assigns a PianoHandler pointer for processing note events.
 *
 * When this member is used to assign a PianoHandler instance, then
 * the methods in that instance are called instead of emitting the
 * signals noteOn() and noteOff().
 * @param handler pointer to a PianoHandler instance
 */
void PianoScene::setPianoHandler(PianoHandler *handler)
{
    d->m_handler = handler;
}

/**
 * Returns the palette used for highlighting the played keys
 * @return The PianoPalette used to highlight the played keys
 */
PianoPalette PianoScene::getHighlightPalette()
{
    return d->m_hilightPalette;
}

/**
 * Displays the note label over a highligted key
 * @param key the activated key
 */
void PianoScene::displayKeyOn(PianoKey* key)
{
    key->setPressed(true);
    int n = key->getNote() + d->m_baseOctave*12 + d->m_transpose;
    QString s = QString("#%1 (%2)").arg(n).arg(noteName(key));
    emit signalName(s);
    KeyLabel* lbl = dynamic_cast<KeyLabel*>(key->childItems().constFirst());
    if (lbl != nullptr) {
        lbl->setDefaultTextColor(d->m_foregroundPalette.getColor(key->isBlack() ? 3 : 2));
        if (d->m_showLabels == ShowActivated) {
            lbl->setVisible(true);
        }
    }
}

/**
 * Displays highlighted the activated key with the supplied color and note velocity
 * @param key the activated key
 * @param color the highlight color
 * @param vel the MIDI note velocity
 */
void PianoScene::showKeyOn( PianoKey* key, QColor color, int vel )
{
    //qDebug() << Q_FUNC_INFO << key->getNote() << vel << color << d->m_velocityTint;
    if (d->m_velocityTint && (vel >= 0) && (vel < 128) && color.isValid() ) {
        QBrush hilightBrush(color.lighter(200 - vel));
        key->setPressedBrush(hilightBrush);
    } else if (color.isValid()) {
        key->setPressedBrush(color);
    }
    displayKeyOn(key);
}

/**
 * Displays highlighted the activated key with the supplied note velocity
 * @param key the activated key
 * @param vel the MIDI note velocity
 */
void PianoScene::showKeyOn( PianoKey* key, int vel )
{
    setHighlightColorFromPolicy(key, vel);
    displayKeyOn(key);
}

/**
 * Displays as deactivated a key
 * @param key the deactivated key
 * @param vel the MIDI note velocity
 */
void PianoScene::showKeyOff( PianoKey* key, int vel)
{
    Q_UNUSED(vel)
    key->setPressed(false);
    emit signalName(QString());
    KeyLabel* lbl = dynamic_cast<KeyLabel*>(key->childItems().constFirst());
    if (lbl != nullptr) {
        lbl->restoreColor();
        if (d->m_showLabels == ShowActivated) {
            lbl->setVisible(false);
        }
    }
}

/**
 * Displays highlighted the corresponding key for a given MIDI note, with a color and MIDI velocity
 * @param note The MIDI note number
 * @param color The highlight color
 * @param vel The MIDI note velocity
 */
void PianoScene::showNoteOn( const int note, QColor color, int vel )
{
    //qDebug() << Q_FUNC_INFO << note << vel << color;
    int n = note - d->m_baseOctave*12 - d->m_transpose;
    if ((note >= d->m_minNote) && (note <= d->m_maxNote) && d->m_keys.contains(n) && color.isValid())
        showKeyOn(d->m_keys.value(n), color, vel);
}

/**
 * Displays highlighted the corresponding key for a given MIDI note, with MIDI velocity
 * @param note The MIDI note number
 * @param vel The MIDI note velocity
 */
void PianoScene::showNoteOn( const int note, int vel )
{
    //qDebug() << Q_FUNC_INFO << note << vel;
    int n = note - d->m_baseOctave*12 - d->m_transpose;
    if ((note >= d->m_minNote) && (note <= d->m_maxNote) && d->m_keys.contains(n)) {
        showKeyOn(d->m_keys.value(n), vel);
    }
}

/**
 * Displays deactivated the corresponding key for a given MIDI note, with MIDI velocity
 * @param note The MIDI note number
 * @param vel The MIDI note velocity
 */
void PianoScene::showNoteOff( const int note, int vel )
{
    int n = note - d->m_baseOctave*12 - d->m_transpose;
    if ((note >= d->m_minNote) && (note <= d->m_maxNote) && d->m_keys.contains(n)) {
        showKeyOff(d->m_keys.value(n), vel);
    }
}

/**
 * Returns the base octave number.
 * @see setBaseOctave()
 * @return the base octave number
 */
int PianoScene::baseOctave() const { return d->m_baseOctave; }

/**
 * Performs a Note On MIDI event for the given MIDI note number and velocity.
 * If a PianoHandler instance is assigned, its PianoHandler::noteOn() method is called,
 * otherwise the noteOn() signal is triggered.
 * @param note The MIDI note number
 * @param vel The MIDI velocity
 */
void PianoScene::triggerNoteOn( const int note, const int vel )
{
    int n = d->m_baseOctave*12 + note + d->m_transpose;
    if ((n >= d->m_minNote) && (n <= d->m_maxNote)) {
        if (d->m_handler != nullptr) {
            d->m_handler->noteOn(n, vel);
        } else {
            emit noteOn(n, vel);
        }
    }
}

/**
 * Performs a Note Off MIDI event for the given MIDI note number and velocity.
 * If a PianoHandler instance is assigned, its PianoHandler::noteOff() method is called,
 * otherwise the noteOff() signal is triggered.
 * @param note The MIDI note number
 * @param vel The MIDI velocity
 */
void PianoScene::triggerNoteOff( const int note, const int vel )
{
    int n = d->m_baseOctave*12 + note + d->m_transpose;
    if ((n >= d->m_minNote) && (n <= d->m_maxNote)) {
        if (d->m_handler != nullptr) {
            d->m_handler->noteOff(n, vel);
        } else {
            emit noteOff(n, vel);
        }
    }
}

/**
 * Assigns to the given key the highlight color from the active highlight palette
 * and the given MIDI velocity.
 * @param key The given piano key
 * @param vel The MIDI note velocity
 */
void PianoScene::setHighlightColorFromPolicy(PianoKey* key, int vel)
{
    QColor c;
    //qDebug() << Q_FUNC_INFO << key->getNote() << vel << d->m_velocityTint;
    switch (d->m_hilightPalette.paletteId()) {
    case PAL_SINGLE:
        c = d->m_hilightPalette.getColor(0);
        break;
    case PAL_DOUBLE:
        c = d->m_hilightPalette.getColor(key->getType());
        break;
    case PAL_CHANNELS:
        c = d->m_hilightPalette.getColor(d->m_channel);
        break;
    case PAL_HISCALE:
        c = d->m_hilightPalette.getColor(key->getDegree());
        break;
    default:
        return;
    }
    if (c.isValid()) {
        if (d->m_velocityTint && (vel >= 0) && (vel < 128)) {
            QBrush h(c.lighter(200 - vel));
            key->setPressedBrush(h);
        } else {
            key->setPressedBrush(c);
        }
    }
}

/**
 * Produces a MIDI Note On event and highlights the given key
 * @param key The given key
 */
void PianoScene::keyOn( PianoKey* key )
{
    triggerNoteOn(key->getNote(), d->m_velocity);
    showKeyOn(key, d->m_velocity);
}

/**
 * Produces a MIDI Note Off event and deactivates the given key
 * @param key The given key
 */
void PianoScene::keyOff( PianoKey* key )
{
    triggerNoteOff(key->getNote(), 0);
    showKeyOff(key, 0);
}

/**
 * Produces a MIDI Note On event and highlights the given key with the given pressure
 * @param key The given key
 * @param pressure The applied pressure
 */
void PianoScene::keyOn( PianoKey* key, qreal pressure )
{
    int vel = d->m_velocity * pressure;
    triggerNoteOn(key->getNote(), vel);
    showKeyOn(key, vel);
}

/**
 * Produces a MIDI Note Off event and deactivates the given key with the given pressure.
 * @param key The given key
 * @param pressure The applied pressure
 */
void PianoScene::keyOff( PianoKey* key, qreal pressure )
{
    int vel = d->m_velocity * pressure;
    triggerNoteOff(key->getNote(), vel);
    showKeyOff(key, vel);
}

/**
 * Produces a MIDI Note On event and highlights the corresponding key for the given MIDI note number.
 * @param note The given MIDI note number
 */
void PianoScene::keyOn(const int note)
{
    if (d->m_keys.contains(note))
        keyOn(d->m_keys.value(note));
    else
        triggerNoteOn(note, d->m_velocity);
}

/**
 * Produces a MIDI Note Off event and deactivates the corresponding key for the given MIDI note number.
 * @param note The given MIDI note number
 */
void PianoScene::keyOff(const int note)
{
    if (d->m_keys.contains(note))
        keyOff(d->m_keys.value(note));
    else
        triggerNoteOff(note, d->m_velocity);
}

/**
 * Returns whether the low level computer keyboard mode is enabled.
 * @return true if the low level computer keyboard mode is enabled
 */
bool PianoScene::getRawKeyboardMode() const
{
    return d->m_rawkbd;
}

/**
 * Returns the piano key for the given scene point coordenates.
 * @param p The given scene point coordenates
 * @return
 */
PianoKey* PianoScene::getKeyForPos( const QPointF& p ) const
{
    PianoKey* key = nullptr;
    QList<QGraphicsItem *> ptitems = this->items(p, Qt::IntersectsItemShape, Qt::DescendingOrder);
    foreach(QGraphicsItem *itm, ptitems) {
        key = dynamic_cast<PianoKey*>(itm);
        if (key != nullptr)
            break;
    }
    return key;
}

/**
 * This event handler, for event mouseEvent, is reimplemented to receive mouse move events for the scene.
 * @param mouseEvent The mouse move event object pointer
 */
void PianoScene::mouseMoveEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
    if (d->m_mouseEnabled) {
        if (d->m_mousePressed) {
            PianoKey* key = getKeyForPos(mouseEvent->scenePos());
            PianoKey* lastkey = getKeyForPos(mouseEvent->lastScenePos());
            if ((lastkey != nullptr) && (lastkey != key) && lastkey->isPressed()) {
                keyOff(lastkey);
            }
            if ((key != nullptr) && !key->isPressed()) {
                keyOn(key);
            }
            mouseEvent->accept();
            return;
        }
    }
}

/**
 * This event handler, for event mouseEvent, is reimplemented to receive mouse press events for the scene.
 * @param mouseEvent The mouse press event object pointer
 */
void PianoScene::mousePressEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
    if (d->m_mouseEnabled) {
        PianoKey* key = getKeyForPos(mouseEvent->scenePos());
        if (key != nullptr && !key->isPressed()) {
            keyOn(key);
            d->m_mousePressed = true;
            mouseEvent->accept();
            return;
        }
    }
}

/**
 * This event handler, for event mouseEvent, is reimplemented to receive mouse release events for the scene.
 * @param mouseEvent The mouse release event object pointer
 */
void PianoScene::mouseReleaseEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
    if (d->m_mouseEnabled) {
        d->m_mousePressed = false;
        PianoKey* key = getKeyForPos(mouseEvent->scenePos());
        if (key != nullptr && key->isPressed()) {
            keyOff(key);
            mouseEvent->accept();
            return;
        }
    }
}

/**
 * Returns the note number for the given computer keyboard key code.
 * @param key The given computer keyboard key code
 * @return The note number
 */
int PianoScene::getNoteFromKey( const int key ) const
{
    if (d->m_keybdMap != nullptr) {
        KeyboardMap::ConstIterator it = d->m_keybdMap->constFind(key);
        if ((it != d->m_keybdMap->constEnd()) && (it.key() == key)) {
            int note = it.value();
            return note;
        }
    }
    return -1;
}

/**
 * Returns the piano key object corresponding to the given computer keyboard key.
 * @param key The given computer keyboard key
 * @return The Piano Key object pointer
 */
PianoKey* PianoScene::getPianoKey( const int key ) const
{
    int note = getNoteFromKey(key);
    if (d->m_keys.contains(note))
        return d->m_keys.value(note);
    return nullptr;
}

/**
 * This event handler, for event keyEvent, is reimplemented to receive keypress events.
 * @param keyEvent The computer keyboard pressed event
 */
void PianoScene::keyPressEvent ( QKeyEvent * keyEvent )
{
    if ( d->m_keyboardEnabled && !keyEvent->isAutoRepeat()) { // ignore auto-repeats
        int note = getNoteFromKey(d->m_rawkbd ? keyEvent->nativeScanCode() : keyEvent->key());
        if (note > -1) {
            keyOn(note);
            keyEvent->accept();
            return;
        }
    }
    keyEvent->ignore();
}

/**
 * This event handler, for event keyEvent, is reimplemented to receive key release events.
 * @param keyEvent The computer keyboard released event
 */
void PianoScene::keyReleaseEvent ( QKeyEvent * keyEvent )
{
    if (d->m_keyboardEnabled && !keyEvent->isAutoRepeat() ) { // ignore auto-repeats
        int note = getNoteFromKey(d->m_rawkbd ? keyEvent->nativeScanCode() : keyEvent->key());
        if (note > -1) {
            keyOff(note);
            keyEvent->accept();
            return;
        }
    }
    keyEvent->ignore();
}

/**
 * Processes touch screen events
 * @param event The given event
 * @return true if the event was processed
 */
bool PianoScene::event(QEvent *event)
{
    switch(event->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchEnd:
    case QEvent::TouchUpdate:
    {
        QTouchEvent *touchEvent = static_cast<QTouchEvent*>(event);
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
        const auto touchScreen = QTouchDevice::DeviceType::TouchScreen;
#else
        const auto touchScreen = QInputDevice::DeviceType::TouchScreen;
#endif
        if (d->m_touchEnabled && touchEvent->device()->type() == touchScreen) {
            QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
            bool hasPressure =
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
                        touchEvent->device()->capabilities().testFlag(QTouchDevice::Pressure);
#else
                        touchEvent->device()->capabilities().testFlag(QInputDevice::Capability::Pressure);
#endif
            foreach(const QTouchEvent::TouchPoint& touchPoint, touchPoints) {
                switch (touchPoint.state()) {
                //case Qt::TouchPointPrimary:
                case Qt::TouchPointStationary:
                    continue;
                case Qt::TouchPointReleased: {
                        PianoKey* key = getKeyForPos(touchPoint.scenePos());
                        if (key != nullptr && key->isPressed()) {
                            if (hasPressure) {
                                keyOff(key, touchPoint.pressure());
                            } else {
                                keyOff(key);
                            }
                        }
                        break;
                    }
                case Qt::TouchPointPressed: {
                        PianoKey* key = getKeyForPos(touchPoint.scenePos());
                        if (key != nullptr && !key->isPressed()) {
                            if (hasPressure) {
                                keyOn(key, touchPoint.pressure());
                            } else {
                                keyOn(key);
                            }
                            key->ensureVisible();
                        }
                        break;
                    }
                case Qt::TouchPointMoved: {
                        PianoKey* key = getKeyForPos(touchPoint.scenePos());
                        PianoKey* lastkey = getKeyForPos(touchPoint.lastScenePos());
                        if ((lastkey != nullptr) && (lastkey != key) && lastkey->isPressed()) {
                            if (hasPressure) {
                                keyOff(lastkey, touchPoint.pressure());
                            } else {
                                keyOff(lastkey);
                            }
                        }
                        if ((key != nullptr) && !key->isPressed()) {
                            if (hasPressure) {
                                keyOn(key, touchPoint.pressure());
                            } else {
                                keyOn(key);
                            }
                        }
                        break;
                    }
                default:
                    //qDebug() << "TouchPoint state: " << touchPoint.state();
                    break;
                }
            }
            //qDebug() << "accepted event: " << event;
            event->accept();
            return true;
        }
        break;
    }
    default:
        break;
    }
    //qDebug() << "unprocessed event: " << event;
    return QGraphicsScene::event(event);
}

/**
 * Deactivates all keys.
 */
void PianoScene::allKeysOff()
{
    foreach(PianoKey* key, d->m_keys) {
        key->setPressed(false);
    }
}

/**
 * Assigns a single color for key highlight. This is an alternative to creating a
 * highlight palette with a single color and assigning it.
 * @see setHighlightPalette()
 * @param color Color for key highlight
 */
void PianoScene::setKeyPressedColor(const QColor& color)
{
    if (color.isValid()) {
        d->m_hilightPalette = PianoPalette(PAL_SINGLE);
        d->m_hilightPalette.setColor(0, color);
        QBrush hilightBrush(color);
        for (PianoKey* key : qAsConst(d->m_keys)) {
            key->setPressedBrush(hilightBrush);
        }
    }
}

/**
 * Assigns the default highlight palette colors and assigns it to the scene.
 */
void PianoScene::resetKeyPressedColor()
{
    d->m_hilightPalette.resetColors();
    QBrush hilightBrush(getKeyPressedColor());
    for (PianoKey* key : qAsConst(d->m_keys)) {
        key->setPressedBrush(hilightBrush);
    }
}

/**
 * Returns the minimum MIDI note number that will be displayed.
 * @return the minimum MIDI note number
 */
int PianoScene::getMinNote() const
{
    return d->m_minNote;
}

/**
 * Hides or shows keys
 */
void PianoScene::hideOrShowKeys()
{
    for (PianoKey* key : qAsConst(d->m_keys)) {
        int n = d->m_baseOctave*12 + key->getNote() + d->m_transpose;
        bool b = !(n > d->m_maxNote) && !(n < d->m_minNote);
        key->setVisible(b);
    }
}

/**
 * Assigns the minimum MIDI note number that will be displayed.
 * @param note the minimum MIDI note number
 */
void PianoScene::setMinNote(const int note)
{
    if (d->m_minNote != note) {
        d->m_minNote = note;
        hideOrShowKeys();
    }
}

/**
 * Returns the maximum MIDI note number that will be displayed.
 * @return the maximum MIDI note number
 */
int PianoScene::getMaxNote() const
{
    return d->m_maxNote;
}

/**
 * Assigns the maximum MIDI note number that will be displayed.
 * @param note the maximum MIDI note number
 */
void PianoScene::setMaxNote(const int note)
{
    if (d->m_maxNote != note) {
        d->m_maxNote = note;
        hideOrShowKeys();
    }
}

/**
 * Returns the transpose amount in semitones.
 * @return the transpose amount in semitones
 */
int PianoScene::getTranspose() const
{
    return d->m_transpose;
}

/**
 * Assigns the octave base number
 * @param base the octave base number
 */
void PianoScene::setBaseOctave(const int base)
{ 
    if (d->m_baseOctave != base) {
        d->m_baseOctave = base;
        hideOrShowKeys();
        refreshLabels();
    }
}

/**
 * Returns the number of keys that will be displayed.
 * @return the number of keys
 */
int PianoScene::numKeys() const
{
    return d->m_numKeys;
}

/**
 * Returns the first key number that will be displayed.
 * @return the first key number
 */
int PianoScene::startKey() const
{
    return d->m_startKey;
}

/**
 * Returns whether the given note number is a octave startup note
 * @param note The given note number
 * @return true if the given note number is a octave startup note
 */
bool PianoScene::isOctaveStart(const int note)
{
    return (note + d->m_transpose + 12) % 12 == 0;
}

/**
 * Returns the note name string that will be displayed over a given piano key.
 * @param key The given piano key
 * @return the note name string
 */
QString PianoScene::noteName( PianoKey* key )
{
    Q_ASSERT(key != nullptr);
    int note = key->getNote();
    int num = (note + d->m_transpose + 12) % 12;
    int adj = ((note + d->m_transpose < 0) ? 2 : 1) - d->m_octave + 1;
    int oct = d->m_baseOctave + ((note + d->m_transpose) / 12) - adj;
    if (d->m_noteNames.isEmpty()) {
        QString name;
        if (!d->m_names_f.isEmpty() && !d->m_names_s.isEmpty()) {
            switch(d->m_alterations) {
            case ShowFlats:
                name = d->m_names_f.value(num);
                break;
            case ShowSharps:
                name =  d->m_names_s.value(num);
                break;
            case ShowNothing:
                if (key->isBlack()) {
                    return QString();
                }
                name =  d->m_names_s.value(num);
                break;
            default:
                break;
            }
        }
        if (d->m_octave==OctaveNothing) {
            return name;
        } else {
            return QString("%1%2").arg(name).arg(oct);
        }
    } else {
        if (d->m_noteNames.length() == 128) {
            int n = d->m_baseOctave*12 + note + d->m_transpose;
            //qDebug() << Q_FUNC_INFO << n << note;
            if (n >= 0 && n < d->m_noteNames.length()) {
                return d->m_noteNames.value(n);
            }
        } else if (d->m_noteNames.length() >= 12) {
            if (d->m_octave==OctaveNothing) {
                return d->m_noteNames.value(num);
            } else {
                return QString("%1%2").arg(d->m_noteNames.value(num)).arg(oct);
            }
        }
        return QString();
    }
}

/**
 * Refresh the visibility and other attributes of the labels shown over the piano keys.
 */
void PianoScene::refreshLabels()
{
    for (KeyLabel* lbl : qAsConst(d->m_labels)) {
        PianoKey* key = dynamic_cast<PianoKey*>(lbl->parentItem());
        if (key != nullptr) {
            lbl->setVisible(false);
            lbl->setFont(font());
            lbl->setDefaultTextColor(d->m_foregroundPalette.getColor(key->isBlack() ? 1 : 0));
            lbl->setOrientation(d->m_orientation);
            lbl->setPlainText(noteName(key));
            lbl->adjust();
            lbl->setVisible((d->m_showLabels == ShowAlways) ||
                (d->m_showLabels == ShowMinimum && isOctaveStart(key->getNote())));
        }
    }
}

/**
 * Refresh the background colors of all the piano keys
 */
void PianoScene::refreshKeys()
{
    for (PianoKey* key : qAsConst(d->m_keys)) {
        if (d->m_showColorScale && (d->m_backgroundPalette.paletteId() == PAL_SCALE)) {
            int degree = key->getNote() % 12;
            key->setBrush(d->m_backgroundPalette.getColor(degree));
        } else {
            key->setBrush(d->m_backgroundPalette.getColor(key->isBlack() ? 1 : 0));
        }
        key->setPressed(false);
    }
}

/**
 * Assigns the label visibility policy to the piano keys
 * @see LabelVisibility
 * @param show the new label visibility policy
 */
void PianoScene::setShowLabels(const LabelVisibility show)
{
    //qDebug() << Q_FUNC_INFO << show;
    if (d->m_showLabels != show) {
        d->m_showLabels = show;
        refreshLabels();
    }
}

/**
 * Returns the alterations name policy.
 * @see LabelAlteration, setAlterations()
 * @return the alterations name policy
 */
LabelAlteration PianoScene::alterations() const
{
    return d->m_alterations;
}

/**
 * Assigns the alterations name policy
 * @see LabelAlteration, alterations()
 * @param use the new alterations name policy
 */
void PianoScene::setAlterations(const LabelAlteration use)
{
    if (d->m_alterations != use) {
        d->m_alterations = use;
        refreshLabels();
    }
}

/**
 * Returns the central octave name policy.
 * @return the central octave name policy
 */
LabelCentralOctave PianoScene::getOctave() const
{
    return d->m_octave;
}

/**
 * Assigns the label orientation policy.
 * @param orientation the label orientation policy
 */
void PianoScene::setOrientation(const LabelOrientation orientation)
{
    if (d->m_orientation != orientation) {
        d->m_orientation = orientation;
        refreshLabels();
    }
}

bool PianoScene::isKeyboardEnabled() const
{
    return d->m_keyboardEnabled;
}

void PianoScene::setOctave(const LabelCentralOctave octave)
{
    if (d->m_octave != octave) {
        d->m_octave = octave;
        refreshLabels();
    }
}

LabelOrientation PianoScene::getOrientation() const
{
    return d->m_orientation;
}

/**
 * Assigns the transpose amount in semitones.
 * @param transpose the transpose amount in semitones
 */
void PianoScene::setTranspose(const int transpose)
{
    if (d->m_transpose != transpose && transpose > -12 && transpose < 12) {
        d->m_transpose = transpose;
        hideOrShowKeys();
        refreshLabels();
    }
}

/**
 * Returns the label visibility policy (display note names over the piano keys).
 * @see LabelVisibility, setShowLabels()
 * @return the label visibility policy
 */
LabelVisibility PianoScene::showLabels() const
{
    return d->m_showLabels;
}

/**
 * Assigns the low level computer keyboard mode.
 * @param b the low level computer keyboard mode
 */
void PianoScene::setRawKeyboardMode(bool b)
{
    if (d->m_rawkbd != b) {
        d->m_rawkbd = b;
    }
}

/**
 * Returns the custom note names list.
 * @return the custom note names list
 */
QStringList PianoScene::customNoteNames() const
{
    return d->m_noteNames;
}

/**
 * Returns the standard note names list.
 * @return the standard note names list
 */
QStringList PianoScene::standardNoteNames() const
{
    return d->m_names_s;
}

/**
 * Returns the MIDI note velocity parameter that is assigned to the MIDI OUT notes.
 * @return the MIDI note velocity
 */
int PianoScene::getVelocity()
{
    return d->m_velocity;
}

/**
 * Assigns the MIDI note velocity parameter that is assigned to the MIDI OUT notes.
 * @param velocity the MIDI note velocity
 */
void PianoScene::setVelocity(const int velocity)
{
    d->m_velocity = velocity;
}

/**
 * Returns the MIDI channel that is assigned to the output events, or used to filter
 * the input events (unless MIDI OMNI mode is enabled).
 * @return the MIDI channel
 */
int PianoScene::getChannel() const
{
    return d->m_channel;
}

/**
 * Assigns the MIDI channel that is included into the output events, or used to filter
 * the input events (unless MIDI OMNI mode is enabled).
 * @param channel the MIDI channel
 */
void PianoScene::setChannel(const int channel)
{
    d->m_channel = channel;
}

/**
 * Assigns the list of custom note names, and enables this mode.
 * @param names the list of custom note names
 */
void PianoScene::useCustomNoteNames(const QStringList& names)
{
    //qDebug() << Q_FUNC_INFO << names;
    d->m_noteNames = names;
    refreshLabels();
}

/**
 * Assigns the standard note names, clearing the list of custom note names.
 */
void PianoScene::useStandardNoteNames()
{
    //qDebug() << Q_FUNC_INFO;
    d->m_noteNames.clear();
    refreshLabels();
}

/**
 * Enables or disables the computer keyboard note generation.
 * @param enable the computer keyboard note generation
 */
void PianoScene::setKeyboardEnabled(const bool enable)
{
    if (enable != d->m_keyboardEnabled) {
        d->m_keyboardEnabled = enable;
    }
}

/**
 * Returns whether the computer keyboard note generation is enabled
 * @return true if the computer keyboard note generation is enabled
 */
bool PianoScene::isMouseEnabled() const
{
    return d->m_mouseEnabled;
}

/**
 * Enables or disables the mouse note generation.
 * @param enable the mouse note generation
 */
void PianoScene::setMouseEnabled(const bool enable)
{
    if (enable != d->m_mouseEnabled) {
        d->m_mouseEnabled = enable;
    }
}

/**
 * Returns whether the touch screen note generation is enabled.
 * @return true if the touch screen note generation is enabled
 */
bool PianoScene::isTouchEnabled() const
{
    return d->m_touchEnabled;
}

/**
 * Enables or disables the touch screen note generation.
 * @param enable the touch screen note generation
 */
void PianoScene::setTouchEnabled(const bool enable)
{
    if (enable != d->m_touchEnabled) {
        d->m_touchEnabled = enable;
    }
}

/**
 * Returns whether the velocity parameter of note events is used to influence the highlight key colors.
 * @return whether the velocity parameter of note events is used to influence the highlight key colors
 */
bool PianoScene::velocityTint() const
{
    return d->m_velocityTint;
}

/**
 * Enables or disables the velocity parameter of note events to influence the highlight key colors.
 * @param enable the velocity parameter of note events to influence the highlight key colors
 */
void PianoScene::setVelocityTint(const bool enable)
{
    //qDebug() << Q_FUNC_INFO << enable;
    d->m_velocityTint = enable;
}

/**
 * Retranslates the standard note names
 */
void PianoScene::retranslate()
{
    d->m_names_s = QStringList{
        tr("C"),
        tr("C♯"),
        tr("D"),
        tr("D♯"),
        tr("E"),
        tr("F"),
        tr("F♯"),
        tr("G"),
        tr("G♯"),
        tr("A"),
        tr("A♯"),
        tr("B")};
    d->m_names_f = QStringList{
        tr("C"),
        tr("D♭"),
        tr("D"),
        tr("E♭"),
        tr("E"),
        tr("F"),
        tr("G♭"),
        tr("G"),
        tr("A♭"),
        tr("A"),
        tr("B♭"),
        tr("B")};
    refreshLabels();
}

/**
 * Enables or disables the color scale key background mode.
 * @param show the color scale key background mode
 */
void PianoScene::setShowColorScale(const bool show)
{
    if (d->m_showColorScale != show) {
        d->m_showColorScale = show;
        refreshKeys();
        invalidate();
    }
}

/**
 * Returns the single highlight palette color.
 * @return the single highlight palette color
 */
QColor PianoScene::getKeyPressedColor() const
{
    return d->m_hilightPalette.getColor(0);
}

/**
 * Assigns the active highlight palette.
 * @param p the active highlight palette
 */
void PianoScene::setHighlightPalette( const PianoPalette& p )
{
    if (d->m_hilightPalette != p) {
        d->m_hilightPalette = p;
        refreshKeys();
        invalidate();
    }
}

/**
 * Returns the background palette.
 * @return the background palette
 */
PianoPalette PianoScene::getBackgroundPalette()
{
    return d->m_backgroundPalette;
}

/**
 * Assigns the active background palette.
 * @param p the active background palette
 */
void PianoScene::setBackgroundPalette(const PianoPalette& p )
{
    if (d->m_backgroundPalette != p) {
        d->m_backgroundPalette = p;
        refreshKeys();
        invalidate();
    }
}

/**
 * Returns the active foreground palette.
 * @return the active foreground palette
 */
PianoPalette PianoScene::getForegroundPalette()
{
    return d->m_foregroundPalette;
}

/**
 * Assigns the active foreground palette.
 * @param p the foreground palette
 */
void PianoScene::setForegroundPalette(const PianoPalette &p)
{
    if (d->m_foregroundPalette != p) {
        d->m_foregroundPalette = p;
        refreshLabels();
        invalidate();
    }
}

/**
 * Returns whether the color scale mode is enabled.
 * @return true if the color scale mode is enabled
 */
bool PianoScene::showColorScale() const
{
    return d->m_showColorScale;
}

void PianoScene::setKeyPicture(const bool natural, const QPixmap &pix)
{
    d->m_keyPix[int(natural)] = pix;
    for (PianoKey* key : qAsConst(d->m_keys)) {
        if (key->isBlack() == !natural) {
            key->setPixmap(pix);
        }
    }
}

QPixmap PianoScene::getKeyPicture(const bool natural)
{
    return d->m_keyPix[int(natural)];
}

void PianoScene::setUseKeyPictures(const bool enable)
{
    d->m_useKeyPix = enable;
    for (PianoKey* key : qAsConst(d->m_keys)) {
        key->setUsePixmap(enable);
    }
}

bool PianoScene::getUseKeyPictures() const
{
    return d->m_useKeyPix;
}

void PianoScene::saveData(QByteArray &ba)
{
    d->saveData(ba);
}

void PianoScene::loadData(QByteArray &ba)
{
    d->loadData(ba);
}

} // namespace widgets
} // namespace drumstick
