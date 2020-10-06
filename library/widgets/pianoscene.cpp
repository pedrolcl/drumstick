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
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPalette>
#include <drumstick/pianokeybd.h>
#include <drumstick/pianoscene.h>
#include <qmath.h>

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
        m_foregroundPalette(PianoPalette(PAL_FONT))
    { }
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
};

const int KEYWIDTH = 180;
const int KEYHEIGHT = 720;

static qreal sceneWidth(int keys) {
    return KEYWIDTH * qCeil( keys * 7.0 / 12.0 );
}

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

PianoScene::~PianoScene()
{
    delete d;
}

QSize PianoScene::sizeHint() const
{
    return {static_cast<int>(sceneWidth(d->m_numKeys)), KEYHEIGHT};
}

void PianoScene::setKeyboardMap(KeyboardMap *map)
{
    d->m_keybdMap = map;
}

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

PianoPalette PianoScene::getHighlightPalette()
{
    return d->m_hilightPalette;
}

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

void PianoScene::showKeyOn( PianoKey* key, QColor color, int vel )
{
    if (d->m_velocityTint && vel >= 0 && color.isValid() ) {
        QBrush hilightBrush(color.lighter(200 - vel));
        key->setPressedBrush(hilightBrush);
    }
    displayKeyOn(key);
}

void PianoScene::showKeyOn( PianoKey* key, int vel )
{
    setHighlightColorFromPolicy(key, vel);
    displayKeyOn(key);
}

void PianoScene::showKeyOff( PianoKey* key, int )
{
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

void PianoScene::showNoteOn( const int note, QColor color, int vel )
{
    int n = note - d->m_baseOctave*12 - d->m_transpose;
    if ((note >= d->m_minNote) && (note <= d->m_maxNote) && d->m_keys.contains(n) && color.isValid())
        showKeyOn(d->m_keys.value(n), color, vel);
}

void PianoScene::showNoteOn( const int note, int vel )
{
    int n = note - d->m_baseOctave*12 - d->m_transpose;
    if ((note >= d->m_minNote) && (note <= d->m_maxNote) && d->m_keys.contains(n)) {
        showKeyOn(d->m_keys.value(n), vel);
    }
}

void PianoScene::showNoteOff( const int note, int vel )
{
    int n = note - d->m_baseOctave*12 - d->m_transpose;
    if ((note >= d->m_minNote) && (note <= d->m_maxNote) && d->m_keys.contains(n)) {
        showKeyOff(d->m_keys.value(n), vel);
    }
}

int PianoScene::baseOctave() const { return d->m_baseOctave; }

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

void PianoScene::setHighlightColorFromPolicy(PianoKey* key, int vel)
{
    QColor c;
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
    default:
        return;
    }
    if (c.isValid()) {
        if (d->m_velocityTint) {
            QBrush h(c.lighter(200 - vel));
            key->setPressedBrush(h);
        } else {
            key->setPressedBrush(c);
        }
    }
}

void PianoScene::keyOn( PianoKey* key )
{
    triggerNoteOn(key->getNote(), d->m_velocity);
    showKeyOn(key, d->m_velocity);
}

void PianoScene::keyOff( PianoKey* key )
{
    triggerNoteOff(key->getNote(), 0);
    showKeyOff(key, 0);
}

void PianoScene::keyOn( PianoKey* key, qreal pressure )
{
    int vel = d->m_velocity * pressure;
    triggerNoteOn(key->getNote(), vel);
    showKeyOn(key, vel);
}

void PianoScene::keyOff( PianoKey* key, qreal pressure )
{
    int vel = d->m_velocity * pressure;
    triggerNoteOff(key->getNote(), vel);
    showKeyOff(key, vel);
}

void PianoScene::keyOn(const int note)
{
    if (d->m_keys.contains(note))
        keyOn(d->m_keys.value(note));
    else
        triggerNoteOn(note, d->m_velocity);
}

void PianoScene::keyOff(const int note)
{
    if (d->m_keys.contains(note))
        keyOff(d->m_keys.value(note));
    else
        triggerNoteOff(note, d->m_velocity);
}

bool PianoScene::getRawKeyboardMode() const { return d->m_rawkbd; }

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

PianoKey* PianoScene::getPianoKey( const int key ) const
{
    int note = getNoteFromKey(key);
    if (d->m_keys.contains(note))
        return d->m_keys.value(note);
    return nullptr;
}

void PianoScene::keyPressEvent ( QKeyEvent * keyEvent )
{
    if ( d->m_keyboardEnabled) {
        if ( !d->m_rawkbd && !keyEvent->isAutoRepeat() ) { // ignore auto-repeats
            int note = getNoteFromKey(keyEvent->key());
            if (note > -1)
                keyOn(note);
        }
        keyEvent->accept();
        return;
    }
    keyEvent->ignore();
}

void PianoScene::keyReleaseEvent ( QKeyEvent * keyEvent )
{
    if (d->m_keyboardEnabled) {
        if ( !d->m_rawkbd && !keyEvent->isAutoRepeat() ) { // ignore auto-repeats
            int note = getNoteFromKey(keyEvent->key());
            if (note > -1)
                keyOff(note);
        }
        keyEvent->accept();
        return;
    }
    keyEvent->ignore();
}

bool PianoScene::event(QEvent *event)
{
    switch(event->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchEnd:
    case QEvent::TouchUpdate:
    {
        QTouchEvent *touchEvent = static_cast<QTouchEvent*>(event);
        if (d->m_touchEnabled && touchEvent->device()->type() == QTouchDevice::DeviceType::TouchScreen) {
            QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
            foreach(const QTouchEvent::TouchPoint& touchPoint, touchPoints) {
                switch (touchPoint.state()) {
                //case Qt::TouchPointPrimary:
                case Qt::TouchPointStationary:
                    continue;
                case Qt::TouchPointReleased: {
                        PianoKey* key = getKeyForPos(touchPoint.scenePos());
                        if (key != nullptr && key->isPressed()) {
                            keyOff(key, touchPoint.pressure());
                        }
                        break;
                    }
                case Qt::TouchPointPressed: {
                        PianoKey* key = getKeyForPos(touchPoint.scenePos());
                        if (key != nullptr && !key->isPressed()) {
                            keyOn(key, touchPoint.pressure());
                            key->ensureVisible();
                        }
                        break;
                    }
                case Qt::TouchPointMoved: {
                        PianoKey* key = getKeyForPos(touchPoint.scenePos());
                        PianoKey* lastkey = getKeyForPos(touchPoint.lastScenePos());
                        if ((lastkey != nullptr) && (lastkey != key) && lastkey->isPressed()) {
                            keyOff(lastkey, touchPoint.pressure());
                        }
                        if ((key != nullptr) && !key->isPressed()) {
                            keyOn(key, touchPoint.pressure());
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

void PianoScene::allKeysOff()
{
    foreach(PianoKey* key, d->m_keys) {
        key->setPressed(false);
    }
}

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

void PianoScene::resetKeyPressedColor()
{
    d->m_hilightPalette.resetColors();
    QBrush hilightBrush(getKeyPressedColor());
    for (PianoKey* key : qAsConst(d->m_keys)) {
        key->setPressedBrush(hilightBrush);
    }
}

int PianoScene::getMinNote() const { return d->m_minNote; }

void PianoScene::hideOrShowKeys()
{
    for (PianoKey* key : qAsConst(d->m_keys)) {
        int n = d->m_baseOctave*12 + key->getNote() + d->m_transpose;
        bool b = !(n > d->m_maxNote) && !(n < d->m_minNote);
        key->setVisible(b);
    }
}

void PianoScene::setMinNote(const int note)
{
    if (d->m_minNote != note) {
        d->m_minNote = note;
        hideOrShowKeys();
    }
}

int PianoScene::getMaxNote() const
{
    return d->m_maxNote;
}

void PianoScene::setMaxNote(const int note)
{
    if (d->m_maxNote != note) {
        d->m_maxNote = note;
        hideOrShowKeys();
    }
}

int PianoScene::getTranspose() const { return d->m_transpose; }

void PianoScene::setBaseOctave(const int base)
{ 
    if (d->m_baseOctave != base) {
        d->m_baseOctave = base;
        hideOrShowKeys();
        refreshLabels();
    }
}

int PianoScene::numKeys() const
{
    return d->m_numKeys;
}

int PianoScene::startKey() const
{
    return d->m_startKey;
}

bool PianoScene::isOctaveStart(const int note)
{
    return (note + d->m_transpose + 12) % 12 == 0;
}

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

void PianoScene::setShowLabels(const LabelVisibility show)
{
    //qDebug() << Q_FUNC_INFO << show;
    if (d->m_showLabels != show) {
        d->m_showLabels = show;
        refreshLabels();
    }
}

LabelAlteration PianoScene::alterations() const
{
    return d->m_alterations;
}

void PianoScene::setAlterations(const LabelAlteration use)
{
    if (d->m_alterations != use) {
        d->m_alterations = use;
        refreshLabels();
    }
}

LabelCentralOctave PianoScene::getOctave() const
{
    return d->m_octave;
}

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

void PianoScene::setTranspose(const int transpose)
{
    if (d->m_transpose != transpose && transpose > -12 && transpose < 12) {
        d->m_transpose = transpose;
        hideOrShowKeys();
        refreshLabels();
    }
}

LabelVisibility PianoScene::showLabels() const
{
    return d->m_showLabels;
}

void PianoScene::setRawKeyboardMode(bool b)
{
    if (d->m_rawkbd != b) {
        d->m_rawkbd = b;
    }
}

QStringList PianoScene::customNoteNames() const
{
    return d->m_noteNames;
}

QStringList PianoScene::standardNoteNames() const
{
    return d->m_names_s;
}

int PianoScene::getVelocity()
{
    return d->m_velocity;
}

void PianoScene::setVelocity(const int velocity)
{
    d->m_velocity = velocity;
}

int PianoScene::getChannel() const
{
    return d->m_channel;
}

void PianoScene::setChannel(const int channel)
{
    d->m_channel = channel;
}

void PianoScene::useCustomNoteNames(const QStringList& names)
{
    //qDebug() << Q_FUNC_INFO << names;
    d->m_noteNames = names;
    refreshLabels();
}

void PianoScene::useStandardNoteNames()
{
    //qDebug() << Q_FUNC_INFO;
    d->m_noteNames.clear();
    refreshLabels();
}

void PianoScene::setKeyboardEnabled(const bool enable)
{
    if (enable != d->m_keyboardEnabled) {
        d->m_keyboardEnabled = enable;
    }
}

bool PianoScene::isMouseEnabled() const
{
    return d->m_mouseEnabled;
}

void PianoScene::setMouseEnabled(const bool enable)
{
    if (enable != d->m_mouseEnabled) {
        d->m_mouseEnabled = enable;
    }
}

bool PianoScene::isTouchEnabled() const
{
    return d->m_touchEnabled;
}

void PianoScene::setTouchEnabled(const bool enable)
{
    if (enable != d->m_touchEnabled) {
        d->m_touchEnabled = enable;
    }
}

bool PianoScene::velocityTint() const
{
    return d->m_velocityTint;
}

void PianoScene::setVelocityTint(const bool enable)
{
    d->m_velocityTint = enable;
}

void PianoScene::retranslate()
{
    d->m_names_s = QStringList{
        tr(u8"C"),
        tr(u8"C♯"),
        tr(u8"D"),
        tr(u8"D♯"),
        tr(u8"E"),
        tr(u8"F"),
        tr(u8"F♯"),
        tr(u8"G"),
        tr(u8"G♯"),
        tr(u8"A"),
        tr(u8"A♯"),
        tr(u8"B")};
    d->m_names_f = QStringList{
        tr(u8"C"),
        tr(u8"D♭"),
        tr(u8"D"),
        tr(u8"E♭"),
        tr(u8"E"),
        tr(u8"F"),
        tr(u8"G♭"),
        tr(u8"G"),
        tr(u8"A♭"),
        tr(u8"A"),
        tr(u8"B♭"),
        tr(u8"B")};
    refreshLabels();
}

void PianoScene::setShowColorScale(const bool show)
{
    if (d->m_showColorScale != show) {
        d->m_showColorScale = show;
        refreshKeys();
        invalidate();
    }
}

QColor PianoScene::getKeyPressedColor() const
{
    return d->m_hilightPalette.getColor(0);
}

void PianoScene::setHighlightPalette( const PianoPalette& p )
{
    if (d->m_hilightPalette != p) {
        d->m_hilightPalette = p;
        refreshKeys();
        invalidate();
    }
}

PianoPalette PianoScene::getBackgroundPalette()
{
    return d->m_backgroundPalette;
}

void PianoScene::setBackgroundPalette(const PianoPalette& p )
{
    if (d->m_backgroundPalette != p) {
        d->m_backgroundPalette = p;
        refreshKeys();
        invalidate();
    }
}

PianoPalette PianoScene::getForegroundPalette()
{
    return d->m_foregroundPalette;
}

void PianoScene::setForegroundPalette(const PianoPalette &p)
{
    if (d->m_foregroundPalette != p) {
        d->m_foregroundPalette = p;
        refreshLabels();
        invalidate();
    }
}

bool PianoScene::showColorScale() const
{
    return d->m_showColorScale;
}

} // namespace widgets
} // namespace drumstick
