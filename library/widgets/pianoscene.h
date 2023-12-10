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

#ifndef PIANOSCENE_H_
#define PIANOSCENE_H_

#include <QGraphicsScene>
#include <QScopedPointer>
#include <QHash>
#include <QTouchEvent>
#include <drumstick/pianokeybd.h>
#include <drumstick/pianopalette.h>
#include "pianokey.h"
#include "keylabel.h"

/**
 * @file pianoscene.h
 * PianoScene class declaration
 */

namespace drumstick { namespace widgets {

    /**
     * @addtogroup Widgets
     * @{
     *
     * @class PianoScene
     * The PianoScene class is a QGraphicsScene composed by a number of graphics items:
     * the piano keys.
     */
class PianoScene : public QGraphicsScene
{
    Q_OBJECT
public:
    PianoScene(const int baseOctave,
               const int numKeys,
               const int startKey,
               const QColor &keyPressedColor = QColor(),
               QObject *parent = nullptr);
    ~PianoScene();

    QSize sizeHint() const;
    void setKeyboardMap(KeyboardMap *map);
    KeyboardMap *getKeyboardMap() const;
    PianoHandler *getPianoHandler() const;
    void setPianoHandler(PianoHandler *handler);

    PianoPalette getHighlightPalette();
    void setHighlightPalette(const PianoPalette &p);
    PianoPalette getBackgroundPalette();
    void setBackgroundPalette(const PianoPalette &p);
    PianoPalette getForegroundPalette();
    void setForegroundPalette(const PianoPalette &p);

    bool showColorScale() const;
    void setShowColorScale(const bool show);

    QColor getKeyPressedColor() const;
    void setKeyPressedColor(const QColor &color);
    void resetKeyPressedColor();

    int getMinNote() const;
    void setMinNote(const int note);
    int getMaxNote() const;
    void setMaxNote(const int note);
    int getTranspose() const;
    void setTranspose(const int transpose);

    LabelVisibility showLabels() const;
    void setShowLabels(const LabelVisibility show);
    LabelAlteration alterations() const;
    void setAlterations(const LabelAlteration use);
    LabelCentralOctave getOctave() const;
    void setOctave(const LabelCentralOctave octave);
    LabelOrientation getOrientation() const;
    void setOrientation(const LabelOrientation orientation);

    bool isKeyboardEnabled() const;
    void setKeyboardEnabled(const bool enable);
    bool isMouseEnabled() const;
    void setMouseEnabled(const bool enable);
    bool isTouchEnabled() const;
    void setTouchEnabled(const bool enable);
    bool velocityTint() const;
    void setVelocityTint(const bool enable);
    bool isOctaveStart(const int note);

    void showNoteOn(const int note, QColor color, int vel = -1);
    void showNoteOn(const int note, int vel = -1);
    void showNoteOff(const int note, int vel = -1);
    int baseOctave() const;
    void setBaseOctave(const int base);
    int numKeys() const;
    int startKey() const;
    void allKeysOff();
    void keyOn(const int note);
    void keyOff(const int note);
    bool getRawKeyboardMode() const;
    void setRawKeyboardMode(const bool b);
    void useCustomNoteNames(const QStringList &names);
    void useStandardNoteNames();
    QStringList customNoteNames() const;
    QStringList standardNoteNames() const;
    int getVelocity();
    void setVelocity(const int velocity);
    int getChannel() const;
    void setChannel(const int channel);
    void retranslate();
    void refreshLabels();
    void hideOrShowKeys();
    void refreshKeys();

    void setKeyPicture(const bool natural, const QPixmap &pix);
    QPixmap getKeyPicture(const bool natural);

    void setUseKeyPictures(const bool enable);
    bool getUseKeyPictures() const;

    void saveData(QByteArray &ba);
    void loadData(QByteArray &ba);

    bool touchScreenEvent(QTouchEvent *touchEvent);

    void setUsingNativeFilter(const bool newState);
    bool isUsingNativeFilter() const;

    void setOctaveSubscript(const bool enable);
    bool octaveSubscript() const;

Q_SIGNALS:
    /**
         * This signal is emitted for each Note On MIDI event created using
         * the computer keyboard, mouse or touch screen. It is not emitted if
         * a PianoHandler has been assigned using setPianoHandler().
         * @param n the MIDI note number
         * @param v the MIDI velocity
         */
    void noteOn(int n, int v);
    /**
         * This signal is emitted for each Note Off MIDI event created using
         * the computer keyboard, mouse or touch screen. It is not emitted if
         * a PianoHandler has been assigned using setPianoHandler().
         * @param n the MIDI note number
         * @param v the MIDI velocity
         */
    void noteOff(int n, int v);
    /**
         * signalName is emitted for each note created, and contains a string
         * with the MIDI note number and the note name for each note on event.
         * @param name the MIDI note number and name
         */
    void signalName(const QString &name);

protected:
    void showKeyOn(PianoKey *key, QColor color, int vel);
    void showKeyOn(PianoKey *key, int vel);
    void showKeyOff(PianoKey *key, int vel);
    void displayKeyOn(PianoKey *key);
    void keyOn(PianoKey *key);
    void keyOff(PianoKey *key);
    void keyOn(PianoKey *key, qreal pressure);
    void keyOff(PianoKey *key, qreal pressure);
    PianoKey *getKeyForPos(const QPointF &p) const;
    PianoKey *getPianoKey(const int key) const;
    QString noteName(PianoKey *key);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void keyPressEvent(QKeyEvent *keyEvent) override;
    void keyReleaseEvent(QKeyEvent *keyEvent) override;
    bool event(QEvent *event) override;

    void triggerNoteOn(const int note, const int vel);
    void triggerNoteOff(const int note, const int vel);
    int getNoteFromKey(const int key) const;
    void setHighlightColorFromPolicy(PianoKey *key, const int vel);

private:
    class PianoScenePrivate;
    QScopedPointer<PianoScenePrivate> d;
    };

/** @} */

}} // namespace drumstick::widgets

#endif /*PIANOSCENE_H_*/
