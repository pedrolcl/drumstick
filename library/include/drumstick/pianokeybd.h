/*
    Virtual Piano Widget for Qt
    Copyright (C) 2008-2025, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QGraphicsView>
#include <QScopedPointer>
#include <QGraphicsScene>
#include "macros.h"
#include "pianopalette.h"

/**
 * @file pianokeybd.h
 * Piano Keyboard Widget
 */

#if defined(DRUMSTICK_STATIC)
#define DRUMSTICK_WIDGETS_EXPORT
#else
#if defined(drumstick_widgets_EXPORTS)
#define DRUMSTICK_WIDGETS_EXPORT Q_DECL_EXPORT
#else
#define DRUMSTICK_WIDGETS_EXPORT Q_DECL_IMPORT
#endif
#endif

namespace drumstick { namespace widgets {

    Q_NAMESPACE

    /**
     * @addtogroup Widgets
     * @{
     *
     * @class RawKbdHandler
     * @brief The RawKbdHandler class callbacks
     *
     * RawKbdHandler provides callbacks for low level computer keyboard events
     */
    class RawKbdHandler {
    public:
        virtual ~RawKbdHandler() = default;
        /**
         * @brief handleKeyPressed handles low level computer keyboard press events
         * @param keycode The low level key code pressed
         * @return whether the event has been processed or not
         */
        virtual bool handleKeyPressed(int keycode) = 0;
        /**
         * @brief handleKeyReleased handles low level computer keyboard reelase events
         * @param keycode The low level key code released
         * @return whether the event has been processed or not
         */
        virtual bool handleKeyReleased(int keycode) = 0;
    };

    /**
     * @brief The PianoHandler class callbacks
     *
     * This class provides handler methods for note events. This class must be
     * inherited and implemented by a program using also PianoKeybd to receive the
     * note events generated from a computer keyboard, mouse or touch events. It is
     * provided using the method @ref PianoKeybd::setPianoHandler() and can be retrieved
     * using @ref PianoKeybd::getPianoHandler(). This mechanism is an option alternative
     * to proces signals @ref PianoKeybd::noteOn() and @ref PianoKeybd::noteOff() .
     */
    class PianoHandler
    {
    public:
        virtual ~PianoHandler() = default;
        /**
         * @brief noteOn handles MIDI note on events
         * @param note MIDI note number
         * @param vel MIDI velocity
         */
        virtual void noteOn( const int note, const int vel ) = 0;
        /**
         * @brief noteOff handles MIDI note off events
         * @param note MIDI note number
         * @param vel MIDI velocity
         */
        virtual void noteOff( const int note, const int vel ) = 0;
    };

    /**
     * @brief KeyboardMap
     *
     * KeyboardMap represents a mapping definition to translate from
     * computer keyboard keys and MIDI notes, either for alphanumeric
     * or low level (raw) events.
     */
    typedef QHash<int, int> KeyboardMap;

    extern DRUMSTICK_WIDGETS_EXPORT KeyboardMap g_DefaultKeyMap;    ///< Global Key Map Variable
    extern DRUMSTICK_WIDGETS_EXPORT KeyboardMap g_DefaultRawKeyMap; ///< Global Raw Key Map Variable

    const int DEFAULTSTARTINGKEY = 9;   ///< Default starting key (A)
    const int DEFAULTBASEOCTAVE = 1;    ///< Default base octave
    const int DEFAULTNUMBEROFKEYS = 88; ///< Default number of piano keys

    /**
     * @brief Labels Visibility
     */
    enum LabelVisibility {
        ShowNever,          ///< Don't show note names
        ShowMinimum,        ///< Show only note C names
        ShowActivated,      ///< Show names when notes are activated
        ShowAlways          ///< Show always note names
    };
    Q_ENUM_NS(LabelVisibility)

    /**
     * @brief Labels for Alterations
     */
    enum LabelAlteration {
        ShowSharps,         ///< Show sharps on black keys
        ShowFlats,          ///< Show flats on black keys
        ShowNothing         ///< Do not show names on black keys
    };
    Q_ENUM_NS(LabelAlteration)

    /**
     * @brief Labels Orientation
     */
    enum LabelOrientation {
        HorizontalOrientation,  ///< Show horizontal names
        VerticalOrientation,    ///< Show vertical names
        AutomaticOrientation    ///< Show horizonal or vertical names depending on the size
    };
    Q_ENUM_NS(LabelOrientation)

    /**
     * @brief Labels Naming
     */
    enum LabelNaming {
        StandardNames,          ///< Show standard names
        CustomNamesWithSharps,  ///< Show custom names with sharps
        CustomNamesWithFlats    ///< Show custom names with flats
    };
    Q_ENUM_NS(LabelNaming)

    /**
     * @brief Labels Central Octave
     */
    enum LabelCentralOctave {
        OctaveNothing = -1,     ///< Don't show octave numbers
        OctaveC3,               ///< Central C, MIDI note #60 is C3
        OctaveC4,               ///< Central C, MIDI note #60 is C4
        OctaveC5                ///< Central C, MIDI note #60 is C5
    };
    Q_ENUM_NS(LabelCentralOctave)

    /**
     * @brief The PianoKeybd class
     *
     * This class is a widget providing the look and behavior of a musical piano keyboard.
     * It is implemented as a QGraphicsView displaying the contents of a QGraphicsScene (PianoScene).
     */
    class DRUMSTICK_WIDGETS_EXPORT PianoKeybd : public QGraphicsView, public RawKbdHandler
    {
        Q_OBJECT
        Q_PROPERTY( int baseOctave READ baseOctave WRITE setBaseOctave )
        Q_PROPERTY( int numKeys READ numKeys WRITE setNumKeys )
        Q_PROPERTY( int rotation READ getRotation WRITE setRotation )
        Q_PROPERTY( QColor keyPressedColor READ getKeyPressedColor WRITE setKeyPressedColor )
        Q_PROPERTY( drumstick::widgets::LabelVisibility showLabels READ showLabels WRITE setShowLabels )
        Q_PROPERTY( drumstick::widgets::LabelAlteration alterations READ labelAlterations WRITE setLabelAlterations )
        Q_PROPERTY( drumstick::widgets::LabelOrientation labelOrientation READ labelOrientation WRITE setLabelOrientation )
        Q_PROPERTY( drumstick::widgets::LabelCentralOctave labelOctave READ labelOctave WRITE setLabelOctave )
        Q_PROPERTY( int transpose READ getTranspose WRITE setTranspose )
        Q_PROPERTY( int startKey READ startKey WRITE setStartKey )
        Q_PROPERTY( QFont labelFont READ font WRITE setFont )

#ifndef Q_MOC_RUN
        Q_CLASSINFO("Author", "Pedro Lopez-Cabanillas <plcl@users.sf.net>")
        Q_CLASSINFO("URL", "https://sourceforge.net/projects/drumstick")
        Q_CLASSINFO("Version", QT_STRINGIFY(VERSION))
#endif

    public:
        explicit PianoKeybd(QWidget *parent = nullptr);
        PianoKeybd(const int baseOctave, const int numKeys, const int startKey, QWidget *parent = nullptr);
        virtual ~PianoKeybd();

        void setFont(const QFont &font);
        PianoHandler* getPianoHandler() const;
        void setPianoHandler(PianoHandler* handler);

        PianoPalette getHighlightPalette() const;
        void setHighlightPalette(const PianoPalette& p );
        PianoPalette getBackgroundPalette() const;
        void setBackgroundPalette(const PianoPalette& p );
        PianoPalette getForegroundPalette() const;
        void setForegroundPalette(const PianoPalette& p );

        bool showColorScale() const;
        void setShowColorScale(const bool show);

        void useCustomNoteNames(const QStringList& names);
        void useStandardNoteNames();
        QStringList customNoteNames() const;
        QStringList standardNoteNames() const;
        void retranslate();

        int baseOctave() const;
        void setBaseOctave(const int baseOctave);
        int numKeys() const;
        int startKey() const;
        void setNumKeys(const int numKeys, const int startKey = DEFAULTSTARTINGKEY);
        int getRotation() const;
        void setRotation(int r);
        QColor getKeyPressedColor() const;
        void setKeyPressedColor(const QColor& c);
        void resetKeyPressedColor();
        LabelVisibility showLabels() const;
        void setShowLabels(const LabelVisibility show);
        LabelAlteration labelAlterations() const;
        void setLabelAlterations(const LabelAlteration use);
        LabelOrientation labelOrientation() const;
        void setLabelOrientation(const LabelOrientation orientation);
        LabelCentralOctave labelOctave() const;
        void setLabelOctave(const LabelCentralOctave octave);
        int getTranspose() const;
        void setTranspose(int t);
        int getChannel() const;
        void setChannel(const int c);
        int getVelocity() const;
        void setVelocity(const int v);

        bool isKeyboardEnabled() const;
        void setKeyboardEnabled( const bool enable );
        bool isMouseEnabled() const;
        void setMouseEnabled( const bool enable );
        bool isTouchEnabled() const;
        void setTouchEnabled( const bool enable );
        bool velocityTint() const ;
        void setVelocityTint( const bool enable );
        void allKeysOff();

        QSize sizeHint() const override;
        void setKeyboardMap(KeyboardMap* m);
        KeyboardMap* getKeyboardMap();
        void resetKeyboardMap();
        void setRawKeyboardMap(KeyboardMap* m);
        KeyboardMap* getRawKeyboardMap();
        void resetRawKeyboardMap();
        bool getRawKeyboardMode() const;
        void setRawKeyboardMode(const bool b);

        void showNoteOn( const int note, QColor color, int vel = -1 );
        void showNoteOn( const int note, int vel = -1 );
        void showNoteOff( const int note, int vel = -1 );

        // RawKbdHandler methods
        bool handleKeyPressed(int keycode) override;
        bool handleKeyReleased(int keycode) override;

        void setKeyPicture(const bool natural, const QPixmap& pix);
        QPixmap getKeyPicture(const bool natural);

        void setUseKeyPictures(const bool enable);
        bool getUseKeyPictures() const;

        void setUsingNativeFilter(const bool newState);
        bool isUsingNativeFilter() const;

        void setOctaveSubscript(const bool enable);
        bool octaveSubscript() const;

        void setStartKey(const int startKey);

    Q_SIGNALS:
        /**
         * This signal is emitted for each Note On MIDI event created using
         * the computer keyboard, mouse or touch screen. It is not emitted if
         * a PianoHandler has been assigned using setPianoHandler().
         * @param midiNote the MIDI note number
         * @param vel the MIDI velocity
         */
        void noteOn( int midiNote, int vel );
        /**
         * This signal is emitted for each Note Off MIDI event created using
         * the computer keyboard, mouse or touch screen. It is not emitted if
         * a PianoHandler has been assigned using setPianoHandler().
         * @param midiNote the MIDI note number
         * @param vel the MIDI velocity
         */
        void noteOff( int midiNote, int vel );
        /**
         * signalName is emitted for each note created, and contains a string
         * with the MIDI note number and the note name for each note on event.
         * @param name the MIDI note number and name
         */
        void signalName( const QString& name );

    protected:
        void initialize();
        void initDefaultMap();
        void initScene(int base, int num, int ini, const QColor& c = QColor());
        void resizeEvent(QResizeEvent *event) override;
        bool viewportEvent(QEvent *ev) override;

    private:
        class PianoKeybdPrivate;
        QScopedPointer<PianoKeybdPrivate> d;
    };

/** @} */

}} // namespace drumstick::widgets

#endif // PIANOKEYBD_H
