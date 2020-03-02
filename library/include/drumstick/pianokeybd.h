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

#ifndef PIANOKEYBD_H
#define PIANOKEYBD_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include "macros.h"

#if defined(VPIANO_PLUGIN)
#include <QtDesigner/QDesignerExportWidget>
#else
#define QDESIGNER_WIDGET_EXPORT DRUMSTICK_EXPORT
#endif

namespace drumstick {
namespace widgets {

    class RawKbdHandler {
    public:
        virtual ~RawKbdHandler() {}
        virtual bool handleKeyPressed(int keycode) = 0;
        virtual bool handleKeyReleased(int keycode) = 0;
    };

    class PianoHandler
    {
    public:
        virtual ~PianoHandler() {}
        virtual void noteOn( const int note, const int vel ) = 0;
        virtual void noteOff( const int note, const int vel ) = 0;
    };

    typedef QHash<int, int> KeyboardMap;

    const int DEFAULTSTARTINGKEY = 9;
    const int DEFAULTBASEOCTAVE = 1;
    const int DEFAULTNUMBEROFKEYS = 88;
    const int KEYLABELFONTSIZE = 7;

    class QDESIGNER_WIDGET_EXPORT PianoKeybd : public QGraphicsView, public RawKbdHandler
    {
        Q_OBJECT
        Q_PROPERTY( int baseOctave READ baseOctave WRITE setBaseOctave )
        Q_PROPERTY( int numKeys READ numKeys WRITE setNumKeys )
        Q_PROPERTY( int rotation READ getRotation WRITE setRotation )
        Q_PROPERTY( QColor keyPressedColor READ getKeyPressedColor WRITE setKeyPressedColor )
        Q_PROPERTY( bool showLabels READ showLabels WRITE setShowLabels )
        Q_PROPERTY( bool useFlats READ useFlats WRITE setUseFlats )
        Q_PROPERTY( int transpose READ getTranspose WRITE setTranspose )
    #if defined(VPIANO_PLUGIN)
        Q_CLASSINFO("Author", "Pedro Lopez-Cabanillas <plcl@users.sf.net>")
        Q_CLASSINFO("URL", "http://sourceforge.net/projects/vmpk")
        Q_CLASSINFO("Version", "1.0")
    #endif
    public:
        PianoKeybd(QWidget *parent = nullptr);
        PianoKeybd(const int baseOctave, const int numKeys, const int startKey, QWidget *parent = nullptr);
        virtual ~PianoKeybd();

        int baseOctave() const;
        void setBaseOctave(const int baseOctave);
        int numKeys() const;
        int startKey() const;
        void setNumKeys(const int numKeys, const int startKey = DEFAULTSTARTINGKEY);
        int getRotation() const;
        void setRotation(int r);
        QColor getKeyPressedColor() const;
        void setKeyPressedColor(const QColor& c);
        bool showLabels() const;
        void setShowLabels(bool show);
        bool useFlats() const;
        void setUseFlats(bool use);
        int getTranspose() const;
        void setTranspose(int t);

        QSize sizeHint() const;
        QGraphicsScene* getPianoScene();
        void setRawKeyboardMap(KeyboardMap* m);
        KeyboardMap* getRawKeyboardMap();
        void resetRawKeyboardMap();
        void resetKeyboardMap();
        void showNoteOn(const int note);
        void showNoteOff(const int note);

        // RawKbdHandler methods
        bool handleKeyPressed(int keycode);
        bool handleKeyReleased(int keycode);

    signals:
        void noteOn( int midiNote, int vel );
        void noteOff( int midiNote, int vel );

    protected:
        void initialize();
        void initDefaultMap();
        void initScene(int base, int num, int ini, const QColor& c = QColor());
        void initSinglePalette();
        void resizeEvent(QResizeEvent *event);

    private:
        class PianoKeybdPrivate;
        PianoKeybdPrivate* d;
    };

} // namespace widgets
} // namespace drumstick

#endif // PIANOKEYBD_H
