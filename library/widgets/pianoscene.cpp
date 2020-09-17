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
#include <QPalette>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
//#include <QDebug>
#include <qmath.h>
#include <drumstick/pianokeybd.h>
#include <drumstick/pianoscene.h>

namespace drumstick { namespace widgets {

#define KEYWIDTH  180
#define KEYHEIGHT 720

static qreal sceneWidth(int keys) {
    return KEYWIDTH * qCeil( keys * 7.0 / 12.0 );
}

PianoScene::PianoScene ( const int baseOctave,
                         const int numKeys,
                         const int startKey,
                         const QColor& keyPressedColor,
                         QObject * parent )
    : QGraphicsScene( QRectF(0, 0, sceneWidth(numKeys), KEYHEIGHT), parent ),
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
    m_handler( 0 ),
    m_showColorScale( false ),
    m_hilightPalette(PianoPalette(PAL_SINGLE)),
    m_backgroundPalette(PianoPalette(PAL_KEYS)),
    m_foregroundPalette(PianoPalette(PAL_FONT))
{
    if (keyPressedColor.isValid()) {
        setKeyPressedColor(keyPressedColor);
    }
    QBrush hilightBrush(getKeyPressedColor());
    PianoKeybd* view = dynamic_cast<PianoKeybd*>(parent);
    if (view != nullptr) {
        setFont(view->font());
    }
    int upperLimit = m_numKeys + m_startKey;
    int adj = m_startKey % 12;
    if (adj >= 5) adj++;
    for(int i = m_startKey; i < upperLimit; ++i)
    {
        float x = 0;
        PianoKey* key = NULL;
        KeyLabel* lbl = NULL;
        int ocs = i / 12 * 7;
        int j = i % 12;
        if (j >= 5) j++;
        if ((j % 2) == 0) {
            x = (ocs + qFloor((j-adj) / 2.0)) * KEYWIDTH;
            key = new PianoKey( QRectF(x, 0, KEYWIDTH, KEYHEIGHT), false, i );
            lbl = new KeyLabel(key);
            lbl->setDefaultTextColor(m_foregroundPalette.getColor(0));
        } else {
            x = (ocs + qFloor((j-adj) / 2.0)) * KEYWIDTH + KEYWIDTH * 6/10 + 1;
            key = new PianoKey( QRectF( x, 0, KEYWIDTH * 8/10 - 1, KEYHEIGHT * 6/10 ), true, i );
            key->setZValue( 1 );
            lbl = new KeyLabel(key);
            lbl->setDefaultTextColor(m_foregroundPalette.getColor(1));
        }
        addItem( key );
        lbl->setFont(font());
        key->setAcceptTouchEvents(true);
        key->setPressedBrush(hilightBrush);
        m_keys.insert(i, key);
        m_labels.insert(i, lbl);
    }
    hideOrShowKeys();
    retranslate();
}

QSize PianoScene::sizeHint() const
{
    return QSize(sceneWidth(m_numKeys), KEYHEIGHT);
}

void PianoScene::displayKeyOn(PianoKey* key)
{
    key->setPressed(true);
    int n = key->getNote() + m_baseOctave*12 + m_transpose;
    QString s = QString("#%1 (%2)").arg(n).arg(noteName(key));
    emit signalName(s);
    KeyLabel* lbl = dynamic_cast<KeyLabel*>(key->childItems().first());
    if (lbl != nullptr) {
        lbl->setDefaultTextColor(m_foregroundPalette.getColor(key->isBlack() ? 3 : 2));
        if (m_showLabels == ShowActivated) {
            lbl->setVisible(true);
        }
    }
}

void PianoScene::showKeyOn( PianoKey* key, QColor color, int vel )
{
    if (m_velocityTint && vel >= 0 && color.isValid() ) {
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
    KeyLabel* lbl = dynamic_cast<KeyLabel*>(key->childItems().first());
    if (lbl != nullptr) {
        lbl->restoreColor();
        if (m_showLabels == ShowActivated) {
            lbl->setVisible(false);
        }
    }
}

void PianoScene::showNoteOn( const int note, QColor color, int vel )
{
    int n = note - m_baseOctave*12 - m_transpose;
    if ((note >= m_minNote) && (note <= m_maxNote) && m_keys.contains(n) && color.isValid())
        showKeyOn(m_keys.value(n), color, vel);
}

void PianoScene::showNoteOn( const int note, int vel )
{
    int n = note - m_baseOctave*12 - m_transpose;
    if ((note >= m_minNote) && (note <= m_maxNote) && m_keys.contains(n)) {
        showKeyOn(m_keys.value(n), vel);
    }
}

void PianoScene::showNoteOff( const int note, int vel )
{
    int n = note - m_baseOctave*12 - m_transpose;
    if ((note >= m_minNote) && (note <= m_maxNote) && m_keys.contains(n)) {
        showKeyOff(m_keys.value(n), vel);
    }
}

void PianoScene::triggerNoteOn( const int note, const int vel )
{
    int n = m_baseOctave*12 + note + m_transpose;
    if ((n >= m_minNote) && (n <= m_maxNote)) {
        if (m_handler != NULL) {
            m_handler->noteOn(n, vel);
        } else {
            emit noteOn(n, vel);
        }
    }
}

void PianoScene::triggerNoteOff( const int note, const int vel )
{
    int n = m_baseOctave*12 + note + m_transpose;
    if ((n >= m_minNote) && (n <= m_maxNote)) {
        if (m_handler != NULL) {
            m_handler->noteOff(n, vel);
        } else {
            emit noteOff(n, vel);
        }
    }
}

void PianoScene::setHighlightColorFromPolicy(PianoKey* key, int vel)
{
    QColor c;
    switch (m_hilightPalette.paletteId()) {
    case PAL_SINGLE:
        c = m_hilightPalette.getColor(0);
        break;
    case PAL_DOUBLE:
        c = m_hilightPalette.getColor(key->getType());
        break;
    case PAL_CHANNELS:
        c = m_hilightPalette.getColor(m_channel);
        break;
    default:
        return;
    }
    if (c.isValid()) {
        if (m_velocityTint) {
            QBrush h(c.lighter(200 - vel));
            key->setPressedBrush(h);
        } else {
            key->setPressedBrush(c);
        }
    }
}

void PianoScene::keyOn( PianoKey* key )
{
    triggerNoteOn(key->getNote(), m_velocity);
    showKeyOn(key, m_velocity);
}

void PianoScene::keyOff( PianoKey* key )
{
    triggerNoteOff(key->getNote(), 0);
    showKeyOff(key, 0);
}

void PianoScene::keyOn( PianoKey* key, qreal pressure )
{
    int vel = m_velocity * pressure;
    triggerNoteOn(key->getNote(), vel);
    showKeyOn(key, vel);
}

void PianoScene::keyOff( PianoKey* key, qreal pressure )
{
    int vel = m_velocity * pressure;
    triggerNoteOff(key->getNote(), vel);
    showKeyOff(key, vel);
}

void PianoScene::keyOn(const int note)
{
    if (m_keys.contains(note))
        keyOn(m_keys.value(note));
    else
        triggerNoteOn(note, m_velocity);
}

void PianoScene::keyOff(const int note)
{
    if (m_keys.contains(note))
        keyOff(m_keys.value(note));
    else
        triggerNoteOff(note, m_velocity);
}

PianoKey* PianoScene::getKeyForPos( const QPointF& p ) const
{
    PianoKey* key = 0;
    QList<QGraphicsItem *> ptitems = this->items(p, Qt::IntersectsItemShape, Qt::DescendingOrder);
    foreach(QGraphicsItem *itm, ptitems) {
        key = dynamic_cast<PianoKey*>(itm);
        if (key != 0)
            break;
    }
    return key;
}

void PianoScene::mouseMoveEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
    if (m_mouseEnabled) {
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
    }
}

void PianoScene::mousePressEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
    if (m_mouseEnabled) {
        PianoKey* key = getKeyForPos(mouseEvent->scenePos());
        if (key != NULL && !key->isPressed()) {
            keyOn(key);
            m_mousePressed = true;
            mouseEvent->accept();
            return;
        }
    }
}

void PianoScene::mouseReleaseEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
    if (m_mouseEnabled) {
        m_mousePressed = false;
        PianoKey* key = getKeyForPos(mouseEvent->scenePos());
        if (key != NULL && key->isPressed()) {
            keyOff(key);
            mouseEvent->accept();
            return;
        }
    }
}

int PianoScene::getNoteFromKey( const int key ) const
{
    if (m_keybdMap != NULL) {
        KeyboardMap::ConstIterator it = m_keybdMap->constFind(key);
        if ((it != m_keybdMap->constEnd()) && (it.key() == key)) {
            int note = it.value();
            return note;
        }
    }
    return -1;
}

PianoKey* PianoScene::getPianoKey( const int key ) const
{
    int note = getNoteFromKey(key);
    if (m_keys.contains(note))
        return m_keys.value(note);
    return NULL;
}

void PianoScene::keyPressEvent ( QKeyEvent * keyEvent )
{
    if ( m_keyboardEnabled) {
        if ( !m_rawkbd && !keyEvent->isAutoRepeat() ) { // ignore auto-repeats
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
    if (m_keyboardEnabled) {
        if ( !m_rawkbd && !keyEvent->isAutoRepeat() ) { // ignore auto-repeats
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
        if (m_touchEnabled && touchEvent->device()->type() == QTouchDevice::DeviceType::TouchScreen) {
            QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
            foreach(const QTouchEvent::TouchPoint& touchPoint, touchPoints) {
                switch (touchPoint.state()) {
                //case Qt::TouchPointPrimary:
                case Qt::TouchPointStationary:
                    continue;
                case Qt::TouchPointReleased: {
                        PianoKey* key = getKeyForPos(touchPoint.scenePos());
                        if (key != NULL && key->isPressed()) {
                            keyOff(key, touchPoint.pressure());
                        }
                        break;
                    }
                case Qt::TouchPointPressed: {
                        PianoKey* key = getKeyForPos(touchPoint.scenePos());
                        if (key != NULL && !key->isPressed()) {
                            keyOn(key, touchPoint.pressure());
                            key->ensureVisible();
                        }
                        break;
                    }
                case Qt::TouchPointMoved: {
                        PianoKey* key = getKeyForPos(touchPoint.scenePos());
                        PianoKey* lastkey = getKeyForPos(touchPoint.lastScenePos());
                        if ((lastkey != NULL) && (lastkey != key) && lastkey->isPressed()) {
                            keyOff(lastkey, touchPoint.pressure());
                        }
                        if ((key != NULL) && !key->isPressed()) {
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
    foreach(PianoKey* key, m_keys) {
        key->setPressed(false);
    }
}

void PianoScene::setKeyPressedColor(const QColor& color)
{
    if (color.isValid()) {
        m_hilightPalette = PianoPalette(PAL_SINGLE);
        m_hilightPalette.setColor(0, color);
        QBrush hilightBrush(color);
        for (PianoKey* key : m_keys) {
            key->setPressedBrush(hilightBrush);
        }
    }
}

void PianoScene::resetKeyPressedColor()
{
    m_hilightPalette.resetColors();
    QBrush hilightBrush(getKeyPressedColor());
    for (PianoKey* key : m_keys) {
        key->setPressedBrush(hilightBrush);
    }
}

void PianoScene::hideOrShowKeys()
{
    for (PianoKey* key : m_keys) {
        int n = m_baseOctave*12 + key->getNote() + m_transpose;
        bool b = !(n > m_maxNote) && !(n < m_minNote);
        key->setVisible(b);
    }
}

void PianoScene::setMinNote(const int note)
{
    if (m_minNote != note) {
        m_minNote = note;
        hideOrShowKeys();
    }
}

void PianoScene::setMaxNote(const int note)
{
    if (m_maxNote != note) {
        m_maxNote = note;
        hideOrShowKeys();
    }
}

void PianoScene::setBaseOctave(const int base)
{ 
    if (m_baseOctave != base) {
        m_baseOctave = base;
        hideOrShowKeys();
        refreshLabels();
    }
}

bool PianoScene::isOctaveStart(const int note)
{
    return (note + m_transpose + 12) % 12 == 0;
}

QString PianoScene::noteName( PianoKey* key )
{
    Q_ASSERT(key != nullptr);
    int note = key->getNote();
    int num = (note + m_transpose + 12) % 12;
    int adj = ((note + m_transpose < 0) ? 2 : 1) - m_octave + 1;
    int oct = m_baseOctave + ((note + m_transpose) / 12) - adj;
    if (m_noteNames.isEmpty()) {
        QString name;
        if (!m_names_f.isEmpty() && !m_names_s.isEmpty()) {
            switch(m_alterations) {
            case ShowFlats:
                name = m_names_f.value(num);
                break;
            case ShowSharps:
                name =  m_names_s.value(num);
                break;
            case ShowNothing:
                if (key->isBlack()) {
                    return QString();
                }
                name =  m_names_s.value(num);
                break;
            default:
                break;
            }
        }
        if (m_octave==Nothing) {
            return name;
        } else {
            return QString("%1%2").arg(name).arg(oct);
        }
    } else {
        if (m_noteNames.length() == 128) {
            int n = m_baseOctave*12 + note + m_transpose;
            //qDebug() << Q_FUNC_INFO << n << note;
            if (n >= 0 && n < m_noteNames.length()) {
                return m_noteNames.value(n);
            }
        } else if (m_noteNames.length() >= 12) {
            if (m_octave==Nothing) {
                return m_noteNames.value(num);
            } else {
                return QString("%1%2").arg(m_noteNames.value(num)).arg(oct);
            }
        }
        return QString();
    }
}

void PianoScene::refreshLabels()
{
    for (KeyLabel* lbl : m_labels) {
        PianoKey* key = dynamic_cast<PianoKey*>(lbl->parentItem());
        if (key != nullptr) {
            lbl->setVisible(false);
            lbl->setFont(font());
            lbl->setDefaultTextColor(m_foregroundPalette.getColor(key->isBlack() ? 1 : 0));
            lbl->setOrientation(m_orientation);
            lbl->setPlainText(noteName(key));
            lbl->adjust();
            lbl->setVisible((m_showLabels == ShowAlways) ||
                (m_showLabels == ShowMinimum && isOctaveStart(key->getNote())));
        }
    }
}

void PianoScene::refreshKeys()
{
    for (PianoKey* key : m_keys) {
        if (m_showColorScale && (m_backgroundPalette.paletteId() == PAL_SCALE)) {
            int degree = key->getNote() % 12;
            key->setBrush(m_backgroundPalette.getColor(degree));
        } else {
            key->setBrush(m_backgroundPalette.getColor(key->isBlack() ? 1 : 0));
        }
        key->setPressed(false);
    }
}

void PianoScene::setShowLabels(const LabelVisibility show)
{
    //qDebug() << Q_FUNC_INFO << show;
    if (m_showLabels != show) {
        m_showLabels = show;
        refreshLabels();
    }
}

void PianoScene::setAlterations(const LabelAlteration use)
{
    if (m_alterations != use) {
        m_alterations = use;
        refreshLabels();
    }
}
void PianoScene::setOrientation(const LabelOrientation orientation)
{
    if (m_orientation != orientation) {
        m_orientation = orientation;
        refreshLabels();
    }
}

void PianoScene::setOctave(const LabelCentralOctave octave)
{
    if (m_octave != octave) {
        m_octave = octave;
        refreshLabels();
    }
}

void PianoScene::setTranspose(const int transpose)
{
    if (m_transpose != transpose && transpose > -12 && transpose < 12) {
        m_transpose = transpose;
        hideOrShowKeys();
        refreshLabels();
    }
}

void PianoScene::setRawKeyboardMode(bool b)
{
    if (m_rawkbd != b) {
        m_rawkbd = b;
    }
}

QStringList PianoScene::customNoteNames() const
{
    return m_noteNames;
}

QStringList PianoScene::standardNoteNames() const
{
    return m_names_s;
}

void PianoScene::useCustomNoteNames(const QStringList& names)
{
    //qDebug() << Q_FUNC_INFO << names;
    m_noteNames = names;
    refreshLabels();
}

void PianoScene::useStandardNoteNames()
{
    //qDebug() << Q_FUNC_INFO;
    m_noteNames.clear();
    refreshLabels();
}

void PianoScene::setKeyboardEnabled(const bool enable)
{
    if (enable != m_keyboardEnabled) {
        m_keyboardEnabled = enable;
    }
}

void PianoScene::setMouseEnabled(const bool enable)
{
    if (enable != m_mouseEnabled) {
        m_mouseEnabled = enable;
    }
}

void PianoScene::setTouchEnabled(const bool enable)
{
    if (enable != m_touchEnabled) {
        m_touchEnabled = enable;
    }
}

void PianoScene::retranslate()
{
    m_names_s = QStringList{
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
    m_names_f = QStringList{
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

void PianoScene::setShowColorScale(const bool show)
{
    if (m_showColorScale != show) {
        m_showColorScale = show;
        refreshKeys();
        invalidate();
    }
}

QColor PianoScene::getKeyPressedColor() const
{
    return m_hilightPalette.getColor(0);
}

void PianoScene::setHighlightPalette( const PianoPalette& p )
{
    if (m_hilightPalette != p) {
        m_hilightPalette = p;
        refreshKeys();
        invalidate();
    }
}

void PianoScene::setBackgroundPalette(const PianoPalette& p )
{
    if (m_backgroundPalette != p) {
        m_backgroundPalette = p;
        refreshKeys();
        invalidate();
    }
}

void PianoScene::setForegroundPalette(const PianoPalette &p)
{
    if (m_foregroundPalette != p) {
        m_foregroundPalette = p;
        refreshLabels();
        invalidate();
    }
}

}} // namespace drumstick::widgets
