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

#ifndef PIANOSCENE_H_
#define PIANOSCENE_H_

#include <QGraphicsScene>
#include <QHash>
#include <drumstick/pianokeybd.h>
#include <drumstick/pianopalette.h>
#include "pianokey.h"
#include "keylabel.h"

/**
 * @file pianoscene.h
 * MIDI Widgets
 * @addtogroup Widgets MIDI Widgets
 * @{
 */

namespace drumstick { namespace widgets {

    /**
     * @brief The PianoScene class
     */
    class DRUMSTICK_EXPORT PianoScene : public QGraphicsScene
    {
        Q_OBJECT
    public:
        PianoScene ( const int baseOctave,
                     const int numKeys,
                     const int startKey,
                     const QColor& keyPressedColor = QColor(),
                     QObject * parent = 0 );

        QSize sizeHint() const;
        void setKeyboardMap( KeyboardMap* map ) { m_keybdMap = map; }
        KeyboardMap* getKeyboardMap() const { return m_keybdMap; }
        PianoHandler* getPianoHandler() const { return m_handler; }
        void setPianoHandler(PianoHandler* handler) { m_handler = handler; }

        PianoPalette getHighlightPalette() { return m_hilightPalette; }
        void setHighlightPalette( const PianoPalette& p );
        PianoPalette getBackgroundPalette() { return m_backgroundPalette; }
        void setBackgroundPalette( const PianoPalette& p );
        PianoPalette getForegroundPalette() { return m_foregroundPalette; }
        void setForegroundPalette( const PianoPalette& p );

        bool showColorScale() const { return m_showColorScale; }
        void setShowColorScale(const bool show);

        QColor getKeyPressedColor() const;
        void setKeyPressedColor(const QColor& color);
        void resetKeyPressedColor();

        int getMinNote() const { return m_minNote; }
        void setMinNote(const int note);
        int getMaxNote() const { return m_maxNote; }
        void setMaxNote(const int note);
        int getTranspose() const { return m_transpose; }
        void setTranspose(const int transpose);

        LabelVisibility showLabels() const { return m_showLabels; }
        void setShowLabels(const LabelVisibility show);
        LabelAlteration alterations() const { return m_alterations; }
        void setAlterations(const LabelAlteration use);
        LabelCentralOctave getOctave() const { return m_octave; }
        void setOctave(const LabelCentralOctave octave);
        LabelOrientation getOrientation() const { return m_orientation; }
        void setOrientation(const LabelOrientation orientation);

        bool isKeyboardEnabled() const { return m_keyboardEnabled; }
        void setKeyboardEnabled( const bool enable );
        bool isMouseEnabled() const { return m_mouseEnabled; }
        void setMouseEnabled( const bool enable );
        bool isTouchEnabled() const { return m_touchEnabled; }
        void setTouchEnabled( const bool enable );
        bool velocityTint() const { return m_velocityTint; }
        void setVelocityTint( const bool enable ) { m_velocityTint = enable; }
        bool isOctaveStart( const int note );

        void showNoteOn( const int note, QColor color, int vel = -1 );
        void showNoteOn( const int note, int vel = -1 );
        void showNoteOff( const int note, int vel = -1 );
        int baseOctave() const { return m_baseOctave; }
        void setBaseOctave( const int base );
        int numKeys() const { return m_numKeys; }
        int startKey() const { return m_startKey; }
        void allKeysOff();
        void keyOn( const int note );
        void keyOff( const int note );
        bool getRawKeyboardMode() const { return m_rawkbd; }
        void setRawKeyboardMode(const bool b);
        void useCustomNoteNames(const QStringList& names);
        void useStandardNoteNames();
        QStringList customNoteNames() const;
        QStringList standardNoteNames() const;
        int getVelocity() { return m_velocity; }
        void setVelocity(const int velocity) { m_velocity = velocity; }
        int getChannel() const { return m_channel; }
        void setChannel(const int channel) { m_channel = channel; }
        void retranslate();
        void refreshLabels();

    signals:
        void noteOn(int n, int v);
        void noteOff(int n, int v);
        void signalName(const QString& name);

    protected:
        void showKeyOn( PianoKey* key, QColor color, int vel );
        void showKeyOn( PianoKey* key, int vel );
        void showKeyOff( PianoKey* key, int vel );
        void displayKeyOn(PianoKey* key);
        void keyOn( PianoKey* key );
        void keyOff( PianoKey* key );
        void keyOn( PianoKey* key, qreal pressure );
        void keyOff( PianoKey* key, qreal pressure );
        PianoKey* getKeyForPos( const QPointF& p ) const;
        PianoKey* getPianoKey( const int key ) const;
        QString noteName( PianoKey* key );
        void mouseMoveEvent ( QGraphicsSceneMouseEvent * mouseEvent );
        void mousePressEvent ( QGraphicsSceneMouseEvent * mouseEvent );
        void mouseReleaseEvent ( QGraphicsSceneMouseEvent * mouseEvent );
        void keyPressEvent ( QKeyEvent * keyEvent );
        void keyReleaseEvent ( QKeyEvent * keyEvent );
        bool event(QEvent *event);

    private:
        void triggerNoteOn( const int note, const int vel );
        void triggerNoteOff( const int note, const int vel );
        int getNoteFromKey( const int key ) const;
        void setHighlightColorFromPolicy(PianoKey* key, const int vel);
        void hideOrShowKeys();
        void refreshKeys();

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
        PianoHandler* m_handler;
        KeyboardMap* m_keybdMap;
        QHash<int, PianoKey*> m_keys;
        QMap<int, KeyLabel*> m_labels;
        QStringList m_noteNames;
        QStringList m_names_s;
        QStringList m_names_f;
        bool m_showColorScale;
        PianoPalette m_hilightPalette;
        PianoPalette m_backgroundPalette;
        PianoPalette m_foregroundPalette;
    };

}} // namespace drumstick::widgets

/** @} */

#endif /*PIANOSCENE_H_*/
